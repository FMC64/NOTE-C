
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

static COperator COperator_procCall_create(const char *proc_name)
{
	COperator res;

	res.type = COPERATOR_PROC_CALL;
	res.data = strdup(proc_name);
	res.ctx = CContext_null_ptr();
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
		CContext_destroy(op.ctx);
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
		return ((CNodeOp*)node.data)->nodeCount == 0;
	case CNODE_VALUE:
		value = node.data;
		if (value->token.type != CTOKEN_BASIC)
			return 0;
		if (CKeyword_resolve(value->token.str, NULL))
			return 1;
		return !str_is_identifier(value->token.str);
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
		printf_error(ctx, "expected value on right of operator %s", str);
		return 0;
	}
	if (CNode_isRawOperator(node)) {
		printf_error_part(ctx, "expected valid value on right of operator %s\ngot: ", str);
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

static int CNodeOp_stripUnaries_lv2_custom(CNodeOp *node_op, const COperatorType_Binding *bindings)
{
	size_t i;
	size_t i_norm;
	CNodeOp *cast;
	CNodeValue *value;
	COperator op;

	for (i_norm = 0; i_norm < node_op->nodeCount; i_norm++) {
		i = node_op->nodeCount - 1 - i_norm;
		CNodeOp_print(*node_op);
		printf("\n");
		if (node_op->node[i].type == CNODE_OP) {
			cast = node_op->node[i].data;
			if (cast->op.type == COPERATOR_CAST && cast->nodeCount == 0) {
				if (!CNodeOp_isUnaryAdequate(node_op, i))
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
				if (!CNodeOp_isUnaryValid(node_op, i))
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
		CNodeOp_print(*op);
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
				if (!CNodeOp_stripToNode(scope, &res, &to_add))
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
						printf_error(CStream_lastCtx(scope->stream), "expected ) to end type cast");
						goto CNode_poll_ac_err;
					}
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
		if (!CNodeOp_stripToNode(scope, &res, &to_add))
			goto CNode_poll_ac_err;
		res = CNodeOp_init(COperator_init(COPERATOR_NONE));
		CNodeOp_addNode(&fun, to_add);
		if (!CNodeOp_stripToNode(scope, &fun, pres))
			goto CNode_poll_ac_err;
	} else if (!CNodeOp_stripToNode(scope, &res, pres))
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
