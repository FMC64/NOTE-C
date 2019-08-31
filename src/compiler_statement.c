
#include "headers.h"

static COperator COperator_init(COperatorType type)
{
	COperator res;

	res.type = type;
	res.data = NULL;
	res.ctx = CContext_null_ptr();
	return res;
}

static COperator COperator_cast_create(CType type)
{
	COperator res;

	res.type = COPERATOR_CAST;
	res.data = CType_alloc(type);
	res.ctx = CContext_null_ptr();
	return res;
}

static COperator COperator_procCall_create(void)
{
	COperator res;

	res.type = COPERATOR_PROC_CALL;
	res.data = NULL;
	res.ctx = CContext_null_ptr();
	return res;
}

static void COperator_print(COperator op)
{
	switch (op.type) {
	case COPERATOR_PROC_CALL:
		printf("call ");
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
		CContext_destroy(op.ctx);
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

static int CNodeOp_tryPrintAsso(CNodeOp op)
{
	const struct {COperatorType type; const char *str; } table[] = {
		{COPERATOR_MUL, "*"},
		{COPERATOR_DIV, "/"},
		{COPERATOR_MOD, "%"},
		{COPERATOR_ADD, "+"},
		{COPERATOR_ADD, "-"},
		{COPERATOR_SHIFT_LEFT, "<<"},
		{COPERATOR_SHIFT_RIGHT, ">>"},
		{COPERATOR_LESS, "<"},
		{COPERATOR_LESS_EQ, "<="},
		{COPERATOR_GREATER, ">"},
		{COPERATOR_GREATER_EQ, ">="},
		{COPERATOR_EQ, "=="},
		{COPERATOR_NOT_EQ, "!="},
		{COPERATOR_BITWISE_AND, "&"},
		{COPERATOR_BITWISE_XOR, "^"},
		{COPERATOR_BITWISE_OR, "|"},
		{COPERATOR_LOGICAL_AND, "&&"},
		{COPERATOR_LOGICAL_OR, "||"},
		{COPERATOR_COPY, "="},
		{COPERATOR_ADD_COPY, "+="},
		{COPERATOR_SUB_COPY, "-="},
		{COPERATOR_MUL_COPY, "*="},
		{COPERATOR_DIV_COPY, "/="},
		{COPERATOR_MOD_COPY, "%="},
		{COPERATOR_SHIFT_LEFT_COPY, "<<="},
		{COPERATOR_SHIFT_RIGHT_COPY, ">>="},
		{COPERATOR_AND_COPY, "&="},
		{COPERATOR_XOR_COPY, "^="},
		{COPERATOR_OR_COPY, "|="},
		{COPERATOR_COMMA, ","},
		{COPERATOR_NONE, NULL}};
	size_t i;

	if (op.op.type == COPERATOR_PROC_CALL) {
		CNode_print(op.node[0]);
		printf("(");
		for (i = 1; i < op.nodeCount; i++) {
			CNode_print(op.node[i]);
			if (i < op.nodeCount - 1)
				printf(", ");
		}
		printf(")");
		return 1;
	}
	if (op.op.type == COPERATOR_ARRAY) {
		CNode_print(op.node[0]);
		printf("[");
		CNode_print(op.node[1]);
		printf("]");
		return 1;
	}
	if (op.op.type == COPERATOR_TERNARY) {
		if (op.nodeCount > 1) {
			CNode_print(op.node[0]);
			printf(" ? ");
			CNode_print(op.node[1]);
			printf(" : ");
			CNode_print(op.node[2]);
		} else {
			printf(" ? ");
			CNode_print(op.node[0]);
			printf(" : ");
		}
		return 1;
	}
	for (i = 0; table[i].type != COPERATOR_NONE; i++)
		if (table[i].type == op.op.type) {
			printf("(");
			CNode_print(op.node[0]);
			printf(" %s ", table[i].str);
			CNode_print(op.node[1]);
			printf(")");
			return 1;
		}
	return 0;
}

static int CNodeOp_tryPrintUnary(CNodeOp op)
{
	const struct {COperatorType type; const char *str; } table[] = {
		{COPERATOR_INC_PRE, "++"},
		{COPERATOR_DEC_PRE, "--"},
		{COPERATOR_UNARY_PLUS, "+"},
		{COPERATOR_UNARY_MINUS, "-"},
		{COPERATOR_LOGICAL_NOT, "!"},
		{COPERATOR_BITWISE_NOT, "~"},
		{COPERATOR_DEREF, "*"},
		{COPERATOR_REF, "&"},
		{COPERATOR_SIZEOF, "sizeof "},
		{COPERATOR_NONE, NULL}};
	const struct {COperatorType type; const char *str; } table_post[] = {
		{COPERATOR_INC_POST, "++"},
		{COPERATOR_DEC_POST, "--"},
		{COPERATOR_NONE, NULL}};
	const struct {COperatorType type; const char *str; } table_post_str[] = {
		{COPERATOR_STRUCT_MEMBER, "."},
		{COPERATOR_STRUCT_PTR_MEMBER, "->"},
		{COPERATOR_NONE, NULL}};
	size_t i;

	if (op.op.type == COPERATOR_CAST) {
		printf("(");
		CType_print(*(CType*)op.op.data);
		printf(")");
		if (op.nodeCount > 0)
			CNode_print(op.node[0]);
		return 1;
	}
	for (i = 0; table[i].type != COPERATOR_NONE; i++)
		if (table[i].type == op.op.type) {
			printf(table[i].str);
			CNode_print(op.node[0]);
			return 1;
		}
	for (i = 0; table_post[i].type != COPERATOR_NONE; i++)
		if (table_post[i].type == op.op.type) {
			CNode_print(op.node[0]);
			printf(table_post[i].str);
			return 1;
		}
	for (i = 0; table_post_str[i].type != COPERATOR_NONE; i++)
		if (table_post_str[i].type == op.op.type) {
			CNode_print(op.node[0]);
			printf(table_post_str[i].str);
			printf(op.op.data);
			return 1;
		}
	return 0;
}

static void CNodeOp_print(CNodeOp op)
{
	size_t i;

	if (!CNodeOp_tryPrintAsso(op)) {
		if (!CNodeOp_tryPrintUnary(op)) {
			COperator_print(op.op);
			printf(": ");
			for (i = 0; i < op.nodeCount; i++) {
				CNode_print(op.node[i]);
				if (i < op.nodeCount - 1)
					printf(", ");
			}
		}
	}
}

static int CNodeOp_at(CNodeOp *op, size_t ndx, CNode *pres)
{
	if (ndx < op->nodeCount) {
		if (pres != NULL)
			*pres = op->node[ndx];
		return 1;
	} else
		return 0;
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

typedef struct {const char *str; COperatorType op_type;} COperatorType_Binding;

int COperatorType_Binding_resolve(const COperatorType_Binding *bindings, CToken token, COperator *pres)
{
	size_t i = 0;

	if (token.type != CTOKEN_BASIC)
		return 0;
	for (i = 0; bindings[i].str != NULL; i++)
		if (streq(bindings[i].str, token.str)) {
			*pres = COperator_init(bindings[i].op_type);
			return 1;
		}
	return 0;
}

static int CNode_isRawOperator(CNode node)
{
	CNodeValue *value;

	switch (node.type) {
	case CNODE_OP:
		if (((CNodeOp*)node.data)->op.type == COPERATOR_TERNARY)
			return ((CNodeOp*)node.data)->nodeCount <= 1;
		return ((CNodeOp*)node.data)->nodeCount == 0;
	case CNODE_VALUE:
		value = node.data;
		if (value->token.type != CTOKEN_BASIC)
			return 0;
		if (CKeyword_resolve(value->token.str, NULL))
			return 1;
		return !str_is_value(value->token.str);
	default:
		return 0;
	}
}

static int CNodeOp_isUnaryValid(CNodeOp *op, size_t ndx)
{
	CNode node;
	const char *str;
	CContext ctx;

	if (op->node[ndx].type == CNODE_OP) {
		str = "(cast)";
		ctx = ((CNodeOp*)op->node[ndx].data)->op.ctx;
	} else {
		str = ((CNodeValue*)op->node[ndx].data)->token.str;
		ctx = ((CNodeValue*)op->node[ndx].data)->token.ctx;
	}
	if (!CNodeOp_at(op, ndx + 1, &node)) {
		printf_error(ctx, "expected value on right of unary operator %s", str);
		return 0;
	}
	if (CNode_isRawOperator(node)) {
		printf_error_part(ctx, "expected valid value on right of unary operator %s\ngot: ", str);
		CNode_print(node);
		printf("\n\n");
		return 0;
	}
	return 1;
}

static int CNodeOp_isUnaryAdequate(CNodeOp *op, size_t ndx)
{
	CNode node;

	if (!CNodeOp_at(op, ndx - 1, &node))
		return 1;
	if (CNode_isRawOperator(node))
		return 1;
	return 0;
}

static int CNodeOp_isUnaryCastAdequate(CNodeOp *op, size_t ndx)
{
	CNode node;

	if (!CNodeOp_at(op, ndx - 1, &node))
		return 1;
	if (node.type == CNODE_VALUE)
		if (CToken_streq(((CNodeValue*)node.data)->token, "sizeof"))
			return 0;
	if (CNode_isRawOperator(node))
		return 1;
	return 0;
}

static void CNodeOp_stripUnary(CNodeOp *node_op, size_t op_ndx, COperator op)
{
	CNode new_node = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(op)));
	size_t i;

	CNodeOp_addNode(new_node.data, node_op->node[op_ndx + 1]);
	node_op->nodeCount--;
	CNode_destroy(node_op->node[op_ndx]);
	for (i = op_ndx; i < node_op->nodeCount; i++)
		node_op->node[i] = node_op->node[i + 1];
	node_op->node[op_ndx] = new_node;
}

static int CNodeOp_stripUnaries(CNodeOp *node_op, const COperatorType_Binding *bindings)
{
	size_t i;
	size_t i_norm;
	CNodeValue *value;
	COperator op;

	for (i_norm = 0; i_norm < node_op->nodeCount; i_norm++) {
		i = node_op->nodeCount - 1 - i_norm;
		if (node_op->node[i].type == CNODE_VALUE) {
			value = node_op->node[i].data;
			if (COperatorType_Binding_resolve(bindings, value->token, &op)) {
				if (!CNodeOp_isUnaryAdequate(node_op, i))
					continue;
				if (!CNodeOp_isUnaryValid(node_op, i))
					return 0;
				CNodeOp_stripUnary(node_op, i, op);
				i_norm--;
			}
		}
	}
	return 1;
}

static int CNodeOp_isUnarySizeofValid(CNodeOp *op, size_t ndx)
{
	CNode node;
	CContext ctx;

	if (op->node[ndx].type == CNODE_OP)
		ctx = ((CNodeOp*)op->node[ndx].data)->op.ctx;
	else
		ctx = ((CNodeValue*)op->node[ndx].data)->token.ctx;
	printf("\n");
	CNodeOp_print(*op);
	printf("\n");
	if (!CNodeOp_at(op, ndx + 1, &node)) {
		printf_error(ctx, "expected value on right of unary operator sizeof");
		return 0;
	}
	if (node.type == CNODE_OP)
		if (((CNodeOp*)node.data)->op.type == COPERATOR_CAST && ((CNodeOp*)node.data)->nodeCount == 0)
			return 1;
	if (CNode_isRawOperator(node)) {
		printf_error_part(ctx, "expected valid value on right of unary operator sizeof\ngot: ");
		CNode_print(node);
		printf("\n\n");
		return 0;
	}
	return 1;
}


static int CNodeOp_stripUnaries_lv2_custom(CNodeOp *node_op, const COperatorType_Binding *bindings)
{
	size_t i;
	size_t i_norm;
	CNodeOp *cast;
	CNodeValue *value;
	COperator op;

	for (i_norm = 0; i_norm < node_op->nodeCount; i_norm++) {
		i = node_op->nodeCount - 1 - i_norm;
		if (node_op->node[i].type == CNODE_OP) {
			cast = node_op->node[i].data;
			if (cast->op.type == COPERATOR_CAST && cast->nodeCount == 0) {
				if (!CNodeOp_isUnaryCastAdequate(node_op, i))
					continue;
				if (!CNodeOp_isUnaryValid(node_op, i))
					return 0;
				op = cast->op;
				cast->op = COperator_init(COPERATOR_NONE);
				CNodeOp_stripUnary(node_op, i, op);
				i_norm--;
				continue;
			}
		}
		if (node_op->node[i].type == CNODE_VALUE) {
			value = node_op->node[i].data;
			if (CToken_streq(value->token, "sizeof")) {
				if (!CNodeOp_isUnaryAdequate(node_op, i))
					continue;
				if (!CNodeOp_isUnarySizeofValid(node_op, i))
					return 0;
				op = COperator_init(COPERATOR_SIZEOF);
				CNodeOp_stripUnary(node_op, i, op);
				i_norm--;
				continue;
			}
		}
		if (node_op->node[i].type == CNODE_VALUE) {
			value = node_op->node[i].data;
			if (COperatorType_Binding_resolve(bindings, value->token, &op)) {
				if (!CNodeOp_isUnaryAdequate(node_op, i))
					continue;
				if (!CNodeOp_isUnaryValid(node_op, i))
					return 0;
				CNodeOp_stripUnary(node_op, i, op);
				i_norm--;
				continue;
			}
		}
	}
	return 1;
}

static int CNodeOp_isAssociativePartValid(CNodeOp *op, size_t ndx, const char *rel, const char *op_str, CContext ctx)
{
	CNode node;

	if (!CNodeOp_at(op, ndx, &node)) {
		printf_error(ctx, "expected value on %s of operator %s", rel, op_str);
		return 0;
	}
	if (CNode_isRawOperator(node)) {
		printf_error_part(ctx, "expected valid value on %s of operator %s\ngot: ", rel, op_str);
		CNode_print(node);
		printf("\n\n");
		return 0;
	}
	return 1;
}

static int CNodeOp_isAssociativeValid(CNodeOp *op, size_t ndx)
{
	const char *str;
	CContext ctx;

	str = ((CNodeValue*)op->node[ndx].data)->token.str;
	ctx = ((CNodeValue*)op->node[ndx].data)->token.ctx;
	if (!CNodeOp_isAssociativePartValid(op, ndx - 1, "left", str, ctx))
		return 0;
	if (!CNodeOp_isAssociativePartValid(op, ndx + 1, "right", str, ctx))
		return 0;
	return 1;
}

static void CNodeOp_stripAssociative(CNodeOp *node_op, size_t op_ndx, COperator op)
{
	CNode new_node = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(op)));
	size_t i;

	CNodeOp_addNode(new_node.data, node_op->node[op_ndx - 1]);
	CNodeOp_addNode(new_node.data, node_op->node[op_ndx + 1]);
	node_op->nodeCount -= 2;
	CNode_destroy(node_op->node[op_ndx]);
	for (i = op_ndx - 1; i < node_op->nodeCount; i++)
		node_op->node[i] = node_op->node[i + 2];
	node_op->node[op_ndx - 1] = new_node;
}

static void CNodeOp_stripAssociativeNode(CNodeOp *node_op, size_t op_ndx)
{
	CNode new_node;
	CNode buf;
	size_t i;

	CNodeOp_addNode(node_op->node[op_ndx].data, node_op->node[op_ndx - 1]);
	CNodeOp_addNode(node_op->node[op_ndx].data, node_op->node[op_ndx + 1]);
	new_node = node_op->node[op_ndx];
	node_op->nodeCount -= 2;
	for (i = op_ndx - 1; i < node_op->nodeCount; i++)
		node_op->node[i] = node_op->node[i + 2];
	node_op->node[op_ndx - 1] = new_node;
	buf = ((CNodeOp*)node_op->node[op_ndx].data)->node[0];
	((CNodeOp*)node_op->node[op_ndx].data)->node[0] = ((CNodeOp*)node_op->node[op_ndx].data)->node[1];
	((CNodeOp*)node_op->node[op_ndx].data)->node[1] = buf;
}

static int CNodeOp_stripAssociatives(CNodeOp *node_op, const COperatorType_Binding *bindings, int is_norm)
{
	size_t i;
	size_t i_norm;
	CNodeValue *value;
	COperator op;

	for (i_norm = 0; i_norm < node_op->nodeCount; i_norm++) {
		i = is_norm ? i_norm : node_op->nodeCount - 1 - i_norm;
		if (node_op->node[i].type == CNODE_VALUE) {
			value = node_op->node[i].data;
			if (COperatorType_Binding_resolve(bindings, value->token, &op)) {
				if (!CNodeOp_isAssociativeValid(node_op, i))
					return 0;
				CNodeOp_stripAssociative(node_op, i, op);
				i -= 2;
			}
		}
	}
	return 1;
}

static int CNodeOp_stripAssociativeTernaries(CNodeOp *node_op)
{
	size_t i;
	size_t i_norm;
	CNodeOp *op;

	for (i_norm = 0; i_norm < node_op->nodeCount; i_norm++) {
		i = node_op->nodeCount - 1 - i_norm;
		if (node_op->node[i].type == CNODE_OP) {
			if (!CNode_isRawOperator(node_op->node[i]))
				continue;
			op = node_op->node[i].data;
			if (op->op.type != COPERATOR_TERNARY)
				continue;
			if (!CNodeOp_isAssociativeValid(node_op, i))
				return 0;
			CNodeOp_stripAssociativeNode(node_op, i);
			i -= 2;
		}
	}
	return 1;
}

static int CNodeOp_stripToNode(CScope *scope, CNodeOp *op, CNode *pres)	// evaluate content of the node (merging tokens using operators)
{
	// lv1 is parsed on first pass
	const COperatorType_Binding lv2[] = {	// https://en.cppreference.com/w/c/language/operator_precedence
		{"++", COPERATOR_INC_PRE},
		{"--", COPERATOR_DEC_PRE},
		{"+", COPERATOR_UNARY_PLUS},
		{"-", COPERATOR_UNARY_MINUS},
		{"!", COPERATOR_LOGICAL_NOT},
		{"*", COPERATOR_DEREF},
		{"&", COPERATOR_REF},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv3[] = {
		{"*", COPERATOR_MUL},
		{"/", COPERATOR_DIV},
		{"%", COPERATOR_MOD},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv4[] = {
		{"+", COPERATOR_ADD},
		{"-", COPERATOR_SUB},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv5[] = {
		{"<<", COPERATOR_SHIFT_LEFT},
		{">>", COPERATOR_SHIFT_RIGHT},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv6[] = {
		{"<", COPERATOR_LESS},
		{"<=", COPERATOR_LESS_EQ},
		{">", COPERATOR_GREATER},
		{">=", COPERATOR_GREATER_EQ},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv7[] = {
		{"==", COPERATOR_EQ},
		{"!=", COPERATOR_NOT_EQ},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv8[] = {
		{"&", COPERATOR_BITWISE_AND},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv9[] = {
		{"^", COPERATOR_BITWISE_XOR},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv10[] = {
		{"|", COPERATOR_BITWISE_OR},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv11[] = {
		{"&&", COPERATOR_LOGICAL_AND},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv12[] = {
		{"||", COPERATOR_LOGICAL_OR},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv14[] = {
		{"=", COPERATOR_COPY},
		{"+=", COPERATOR_ADD_COPY},
		{"-=", COPERATOR_SUB_COPY},
		{"*=", COPERATOR_MUL_COPY},
		{"/=", COPERATOR_DIV_COPY},
		{"%=", COPERATOR_MOD_COPY},
		{"<<=", COPERATOR_SHIFT_LEFT_COPY},
		{">>=", COPERATOR_SHIFT_RIGHT_COPY},
		{"&=", COPERATOR_AND_COPY},
		{"^=", COPERATOR_XOR_COPY},
		{"|=", COPERATOR_OR_COPY},
		{NULL, COPERATOR_NONE}};
	const COperatorType_Binding lv15[] = {
		{",", COPERATOR_COMMA},
		{NULL, COPERATOR_NONE}};
	size_t i;

	if (!CNodeOp_stripUnaries_lv2_custom(op, lv2))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv3, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv4, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv5, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv6, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv7, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv8, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv9, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv10, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv11, 1))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv12, 1))
		return 0;
	if (!CNodeOp_stripAssociativeTernaries(op))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv14, 0))
		return 0;
	if (!CNodeOp_stripAssociatives(op, lv15, 1))
		return 0;
	if (op->nodeCount == 0) {
		printf_error(CStream_atCtx(scope->stream), "nothing to evaluate here");
		return 0;
	}
	if (op->nodeCount > 1) {
		printf_error_part(CStream_atCtx(scope->stream), "invalid expression: operators probably missing:\n");	// TODO: print all excess nodes to help programmer figuring out what's wrong
		for (i = 0; i < op->nodeCount; i++) {
			printf("{");
			CNode_print(op->node[i]);
			printf("}");
		}
		printf("\n\n");
		return 0;
	}
	*pres = op->node[0];
	op->nodeCount--;
	CNodeOp_destroy(*op);
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

static int CNode_poll_ac(CScope *scope, const char *sep, CNodeOp *fun, const char *par, int *is_done, size_t depth, CNode *pres)
{
	CNodeOp res = CNodeOp_init(COperator_init(COPERATOR_NONE));	// just a buffer to put temporary tokens, at the end it should contain only one node (which is our response)
	CToken cur;
	CNode to_add_buf;
	CNode to_add;
	int is_last_identifier = 0;
	int is_last_op = 0;
	int has_comma;
	const char *rpars[] = {")", "]", ":", NULL};
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
		if (fun != NULL)
			if (CToken_streq(cur, ",")) {
				if (!CNodeOp_stripToNode(scope, &res, &to_add))
					goto CNode_poll_ac_err;
				res = CNodeOp_init(COperator_init(COPERATOR_NONE));
				CNodeOp_addNode(fun, to_add);
				CStream_forward(scope->stream);
				has_comma = 1;
			}
		if (!has_comma) {
			if (CToken_streq(cur, "(")) {
				CStream_forward(scope->stream);
				if (CStream_atIsType(scope)) {
					to_add = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(COPERATOR_CAST))));
					((CNodeOp*)to_add.data)->op.ctx = CContext_dup(CStream_atCtx(scope->stream));
					if (!CType_parse(scope, &type))
						goto CNode_poll_ac_err;
					((CNodeOp*)to_add.data)->op.data = CType_alloc(type);
					CNodeOp_addNode(&res, to_add);
					if (!CStream_poll(scope->stream, &cur)) {
						printf_error(CStream_lastCtx(scope->stream), "expected ) to end type cast");
						goto CNode_poll_ac_err;
					}
					if (!CToken_streq(cur, ")")) {
						printf_error(cur.ctx, "expected ) to end type cast");
						goto CNode_poll_ac_err;
					}
				} else {
					if (!is_last_op && res.nodeCount > 0) {
						to_add = res.node[res.nodeCount - 1];
						res.node[res.nodeCount - 1] = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(COPERATOR_PROC_CALL))));
						CNodeOp_addNode(res.node[res.nodeCount - 1].data, to_add);
						if (!CNode_poll_ac(scope, sep, res.node[res.nodeCount - 1].data, ")", is_done, depth + 1, &to_add))
							goto CNode_poll_ac_err;
					} else {
						if (!CNode_poll_ac(scope, sep, NULL, ")", is_done, depth + 1, &to_add))
							goto CNode_poll_ac_err;
						CNodeOp_addNode(&res, to_add);
					}
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
			} else if (CToken_streq(cur, "?")) {
				CStream_forward(scope->stream);

				if (!CNode_poll_ac(scope, sep, NULL, ":", is_done, depth + 1, &to_add_buf))
					goto CNode_poll_ac_err;
				to_add = CNode_init(CNODE_OP, CNodeOp_alloc(CNodeOp_init(COperator_init(COPERATOR_TERNARY))));
				CNodeOp_addNode(to_add.data, to_add_buf);
				CNodeOp_addNode(&res, to_add);
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
		is_last_op = CNode_isRawOperator(res.node[res.nodeCount - 1]);
		if (cur.type == CTOKEN_BASIC)
			is_last_identifier = str_is_identifier(cur.str);
		else
			is_last_identifier = 0;
	}
	if (fun != NULL) {
		if (res.nodeCount > 0) {
			if (!CNodeOp_stripToNode(scope, &res, &to_add))
				goto CNode_poll_ac_err;
			res = CNodeOp_init(COperator_init(COPERATOR_NONE));
			CNodeOp_addNode(fun, to_add);
		}
	} else if (!CNodeOp_stripToNode(scope, &res, pres))
		goto CNode_poll_ac_err;
	return 1;

CNode_poll_ac_err:
	CNodeOp_destroy(res);
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
