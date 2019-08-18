
#include "headers.h"

static COperator COperator_init(COperatorType type)
{
	COperator res;

	res.type = type;
	res.data = NULL;
	return res;
}

static COperator COperator_cast_create(CType type)
{
	COperator res;

	res.type = COPERATOR_CAST;
	res.data = CType_alloc(type);
	return res;
}


static void COperator_destroy(COperator op)
{
	if (op.type == COPERATOR_CAST) {
		CType_destroy(*(CType*)op.data);
		free(op.data);
	}
}

static CNode CNode_init(CNodeType type, void *data)
{
	CNode res;

	res.type = type;
	res.data = data;
	return res;
}

static CNodeOp CNodeOp_init(COperator op)
{
	CNodeOp res;

	res.op = op;
	res.nodeCount = 0;
	res.node = NULL;
	return res;
}

static CNodeOp* CNodeOp_alloc(CNodeOp op)
{
	CNodeOp *res = (CNodeOp*)malloc(sizeof(CNodeOp));

	*res = op;
	return res;
}

static void CNodeOp_addNode(CNodeOp *op, CNode to_add)
{
	size_t cur = op->nodeCount++;

	op->node = (CNode*)realloc(op->node, op->nodeCount * sizeof(CNode));
	op->node[cur] = to_add;
}

static void CNodeOp_print(CNodeOp op)
{
	size_t i;

	printf("(op %u: ", op.op.type);
	for (i = 0; i < op.nodeCount; i++) {
		CNode_print(op.node[i]);
		if (i < op.nodeCount - 1)
			printf(", ");
	}
	printf(")");
}

static void CNodeOp_destroy(CNodeOp op)
{
	size_t i;

	COperator_destroy(op.op);
	for (i = 0; i < op.nodeCount; i++)
		CNode_destroy(op.node[i]);
	free(op.node);
}

static CNodeValue CNodeValue_create(CToken token)
{
	CNodeValue res;

	res.token = CToken_dupCtx(token);
	return res;
}

static CNodeValue* CNodeValue_alloc(CNodeValue value)
{
	CNodeValue *res = (CNodeValue*)malloc(sizeof(CNodeValue));

	*res = value;
	return res;
}

static void CNodeValue_print(CNodeValue value)
{
	CToken_print(value.token);
}

static void CNodeValue_destroy(CNodeValue value)
{
	CToken_destroyCtx(value.token);
}

void CNode_destroy(CNode node)
{
	switch (node.type) {
	case CNODE_OP:
		CNodeOp_destroy(*(CNodeOp*)node.data);
		free(node.data);
		return;
	case CNODE_VALUE:
		CNodeValue_destroy(*(CNodeValue*)node.data);
		free(node.data);
		return;
	}
}

static int CNode_poll_ac(CScope *scope, const char *sep, int *is_done, size_t depth, CNode *pres)
{
	CNodeOp res = CNodeOp_init(COperator_init(COPERATOR_NONE));	// just a buffer to put temporary tokens, at the end it should contain only one node (which is our response)
	CToken cur;
	CNode to_add;

	while (!(*is_done) && CStream_at(scope->stream, &cur)) {
		if (CToken_streq(cur, sep)) {
			if (depth != 0) {
				printf_error(cur.ctx, "closing statement with token '%s' but %u %s still left to close", sep, depth, depth > 1 ? "parentheses are" : "parenthesis is");
				CNodeOp_destroy(res);
				return 0;
			}
			if (!CStream_nextBatch(scope->stream)) {
				CNodeOp_destroy(res);
				return 0;
			}
			*is_done = 1;
			break;
		}
		if (CToken_streq(cur, "(")) {
			CStream_forward(scope->stream);
			if (!CNode_poll_ac(scope, sep, is_done, depth + 1, &to_add)) {
				CNodeOp_destroy(res);
				return 0;
			}
		} else if (CToken_streq(cur, ")")) {
			CStream_forward(scope->stream);
			break;
		} else {
			to_add = CNode_init(CNODE_VALUE, CNodeValue_alloc(CNodeValue_create(cur)));
			CStream_forward(scope->stream);
		}
		CNodeOp_addNode(&res, to_add);
	}
	*pres = CNode_init(CNODE_OP, CNodeOp_alloc(res));
	return 1;
}

void CNode_print(CNode node)
{
	switch (node.type) {
	case CNODE_OP:
		CNodeOp_print(*(CNodeOp*)node.data);
		return;
	case CNODE_VALUE:
		CNodeValue_print(*(CNodeValue*)node.data);
		return;
	}
}

int CNode_poll(CScope *scope, const char *sep, CNode *pres)
{
	int is_done = 0;

	if (!CNode_poll_ac(scope, sep, &is_done, 0, pres))
		return 0;
	if (!is_done) {
		printf_error(CStream_atCtx(scope->stream), "excess parenthesis");
		CNode_destroy(*pres);
		return 0;
	}
	return 1;
}
