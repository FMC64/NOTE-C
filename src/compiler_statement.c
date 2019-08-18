
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

static COperator COperator_procCall_create(const char *proc_name)
{
	COperator res;

	res.type = COPERATOR_PROC_CALL;
	res.data = strdup(proc_name);
	return res;
}

static void COperator_print(COperator op)
{
	switch (op.type) {
	case COPERATOR_PROC_CALL:
		printf("call %s", op.data);
		return;
	case COPERATOR_CAST:
		printf("cast ");
		CType_print(*(CType*)op.data);
		return;
	case COPERATOR_STRUCT_MEMBER:
		printf("struct.%s", op.data);
		return;
	case COPERATOR_STRUCT_PTR_MEMBER:
		printf("struct_ptr->%s", op.data);
		return;
	default:
		printf("op %u", op.type);
	}
}

static void COperator_destroy(COperator op)
{
	switch (op.type) {
	case COPERATOR_CAST:
		CType_destroy(*(CType*)op.data);
		free(op.data);
		return;
	case COPERATOR_PROC_CALL:
		free(op.data);
		return;
	case COPERATOR_STRUCT_MEMBER:
	case COPERATOR_STRUCT_PTR_MEMBER:
		free(op.data);
		return;
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

static void CNodeOp_removeLast(CNodeOp *op)
{
	CNode_destroy(op->node[--op->nodeCount]);
}

static void CNodeOp_print(CNodeOp op)
{
	size_t i;

	printf("(");
	COperator_print(op.op);
	printf(": ");
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

static int CNodeOp_stripToNode(CNodeOp op, CNode *pres)	// evaluate content of the node (merging tokens using operators)
{
	*pres = CNode_init(CNODE_OP, CNodeOp_alloc(op));
	return 1;
}

static int CToken_to_unaryOp(CToken cur, COperatorType *pres)
{
	const struct {const char *str; COperatorType op_type;} unary_ops[] = {
		{"++", COPERATOR_INC_POST},
		{"--", COPERATOR_DEC_POST},
		{NULL, COPERATOR_NONE}};
	size_t i;

	if (cur.type != CTOKEN_BASIC)
		return 0;
	for (i = 0; unary_ops[i].str != NULL; i++)
		if (streq(cur.str, unary_ops[i].str)) {
			*pres = unary_ops[i].op_type;
			return 1;
		}
	return 0;
}

static int CToken_to_assoOp(CToken cur, COperatorType *pres)
{
	const struct {const char *str; COperatorType op_type;} asso_ops[] = {
		{".", COPERATOR_STRUCT_MEMBER},
		{"->", COPERATOR_STRUCT_PTR_MEMBER},
		{NULL, COPERATOR_NONE}};
	size_t i;

	if (cur.type != CTOKEN_BASIC)
		return 0;
	for (i = 0; asso_ops[i].str != NULL; i++)
		if (streq(cur.str, asso_ops[i].str)) {
			*pres = asso_ops[i].op_type;
			return 1;
		}
	return 0;
}

static int CNode_poll_ac(CScope *scope, const char *sep, const char *proc_name, const char *par, int *is_done, size_t depth, CNode *pres)
{
	CNodeOp res = CNodeOp_init(COperator_init(COPERATOR_NONE));	// just a buffer to put temporary tokens, at the end it should contain only one node (which is our response)
	CNodeOp fun = CNodeOp_init(COperator_procCall_create(proc_name));  // used if is_proc
	CToken cur;
	CNode to_add;
	int is_last_identifier = 0;
	int is_last_op = 0;
	int has_comma;
	char *sub_proc_name;
	const char *rpars[] = {")", "]", NULL};
	COperatorType op_type;
	CType type;

	while (!(*is_done) && CStream_at(scope->stream, &cur)) {
		if (CToken_streq(cur, sep)) {
			if (depth != 0) {
				printf_error(cur.ctx, "closing statement with token '%s' but %u %s still left to close", sep, depth, depth > 1 ? "parentheses are" : "parenthesis is");
				goto CNode_poll_ac_err;
			}
			if (!CStream_nextBatch(scope->stream))
				goto CNode_poll_ac_err;
			*is_done = 1;
			break;
		}
		has_comma = 0;
		if (proc_name != NULL)
			if (CToken_streq(cur, ",")) {
				if (!CNodeOp_stripToNode(res, &to_add))
					goto CNode_poll_ac_err;
				res = CNodeOp_init(COperator_init(COPERATOR_NONE));
				CNodeOp_addNode(&fun, to_add);
				CStream_forward(scope->stream);
				has_comma = 1;
			}
		if (!has_comma) {
			if (CToken_streq(cur, "(")) {
				CStream_forward(scope->stream);
				if (CStream_atIsType(scope)) {
					to_add = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(COPERATOR_CAST))));
					if (!CType_parse(scope, &type))
						goto CNode_poll_ac_err;
					((CNodeOp*)to_add.data)->op.data = CType_alloc(type);
					CNodeOp_addNode(&res, to_add);
					if (!CStream_poll(scope->stream, &cur)) {
						printf_error(CStream_lastCtx(scope->stream), "expected ) to end type cast");
						goto CNode_poll_ac_err;
					}
					if (!CToken_streq(cur, ")")) {
						printf_error(CStream_lastCtx(scope->stream), "expected ) to end type cast");
						goto CNode_poll_ac_err;
					}
					is_last_identifier = 0;
					is_last_op = 1;
					continue;
				} else {
					if (is_last_identifier) {
						sub_proc_name = strdup(((CNodeValue*)res.node[res.nodeCount - 1].data)->token.str);
						CNodeOp_removeLast(&res);
					} else
						sub_proc_name = NULL;
					if (!CNode_poll_ac(scope, sep, sub_proc_name, ")", is_done, depth + 1, &to_add)) {
						free(sub_proc_name);
						goto CNode_poll_ac_err;
					}
					free(sub_proc_name);
					CNodeOp_addNode(&res, to_add);
				}
			} else if (CToken_streq(cur, "[")) {
				CStream_forward(scope->stream);
				if (res.nodeCount == 0) {
					printf_error(cur.ctx, "expected a token before [ for array subscripting");
					goto CNode_poll_ac_err;
				}
				to_add = res.node[res.nodeCount - 1];
				res.node[res.nodeCount - 1] = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(COPERATOR_ARRAY))));
				CNodeOp_addNode(res.node[res.nodeCount - 1].data, to_add);
				if (!CNode_poll_ac(scope, sep, NULL, "]", is_done, depth + 1, &to_add))
					goto CNode_poll_ac_err;
				CNodeOp_addNode(res.node[res.nodeCount - 1].data, to_add);
			} else if (!is_last_op && res.nodeCount > 0 && CToken_to_unaryOp(cur, &op_type)) {
				CStream_forward(scope->stream);
				to_add = res.node[res.nodeCount - 1];
				res.node[res.nodeCount - 1] = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(op_type))));
				CNodeOp_addNode(res.node[res.nodeCount - 1].data, to_add);
			} else if (CToken_to_assoOp(cur, &op_type)) {
				CStream_forward(scope->stream);
				if (res.nodeCount == 0) {
					printf_error(cur.ctx, "expected token before %s operator", cur.str);
					goto CNode_poll_ac_err;
				}
				if (!CStream_poll(scope->stream, &cur)) {
					printf_error(cur.ctx, "expected token after %s operator", cur.str);
					goto CNode_poll_ac_err;
				}
				if (cur.type != CTOKEN_BASIC) {
					printf_error_part(cur.ctx, "invalid token for member accessing:\n");
					CToken_print(cur);
					printf("\n\n");
					goto CNode_poll_ac_err;
				}
				to_add = res.node[res.nodeCount - 1];
				res.node[res.nodeCount - 1] = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(op_type))));
				CNodeOp_addNode(res.node[res.nodeCount - 1].data, to_add);
				((CNodeOp*)res.node[res.nodeCount - 1].data)->op.data = strdup(cur.str);
			} else if (CToken_streq_in(cur, rpars)) {
				CStream_forward(scope->stream);
				if (!CToken_streq(cur, par)) {
					printf_error(cur.ctx, "illegal parenthesis closure in this context\nexpected %s, got %s", par, cur.str);
					goto CNode_poll_ac_err;
				}
				break;
			} else {
				to_add = CNode_init(CNODE_VALUE, CNodeValue_alloc(CNodeValue_create(cur)));
				CStream_forward(scope->stream);
				CNodeOp_addNode(&res, to_add);
			}
		}
		if (cur.type == CTOKEN_BASIC) {
			is_last_identifier = str_is_identifier(cur.str);
			is_last_op = CToken_streq_in(cur, c_op);
		} else {
			is_last_identifier = 0;
			is_last_op = 0;
		}
	}
	if (proc_name != NULL) {
		if (!CNodeOp_stripToNode(res, &to_add))
			goto CNode_poll_ac_err;
		res = CNodeOp_init(COperator_init(COPERATOR_NONE));
		CNodeOp_addNode(&fun, to_add);
		if (!CNodeOp_stripToNode(fun, pres))
			goto CNode_poll_ac_err;
	} else if (!CNodeOp_stripToNode(res, pres))
		goto CNode_poll_ac_err;
	return 1;

CNode_poll_ac_err:
	CNodeOp_destroy(res);
	CNodeOp_destroy(fun);
	return 0;
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

	if (!CNode_poll_ac(scope, sep, NULL, ")", &is_done, 0, pres))
		return 0;
	if (!is_done) {
		printf_error(CStream_atCtx(scope->stream), "excess parenthesis");
		CNode_destroy(*pres);
		return 0;
	}
	return 1;
}
