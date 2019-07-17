
#include "headers.h"

CReference CReference_default(void)
{
	CReference res;

	res.level = 0;
	res.arrayCount = 0;
	res.array = NULL;
	return res;
}

void CReference_destroy(CReference reference)
{
	free(reference.array);
}

CPrimitive CPrimitive_default(void)
{
	CPrimitive res;

	res.type = CPRIMITIVE_NONE;
	res.data = NULL;
	res.isDataRef = 0;
	return res;
}

void CPrimitive_destroy(CPrimitive primitive)
{
	if (primitive.isDataRef)
		return;
	switch (primitive.type) {
	case CPRIMITIVE_FUNCTION:
		CFunction_destroy(primitive.data);
		break;
	}
}

CType CType_default(void)
{
	CType res;

	res.flags = 0;
	res.ref = CReference_default();
	res.primitive = CPrimitive_default();
	return res;
}

static const struct {CKeyword keyword; CTypeFlag flag;} keyword_flag[] = {
{CKEYWORD_CONST, CTYPE_CONST},
{CKEYWORD_VOLATILE, CTYPE_VOLATILE},
{CKEYWORD_NONE, CTYPE_NONE}};

static CKeyword CTypeFlag_keyword(CTypeFlag flag)
{
	size_t i;

	for (i = 0; keyword_flag[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_flag[i].flag == flag)
			return keyword_flag[i].keyword;
	return CKEYWORD_NONE;
}

const char* CTypeFlag_str(CTypeFlag flag)
{
	return CKeyword_str(CTypeFlag_keyword(flag));
}

static int get_flag(CKeyword keyword, CTypeFlag *flag)
{
	size_t i;

	for (i = 0; keyword_flag[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_flag[i].keyword == keyword) {
			*flag = keyword_flag[i].flag;
			return 1;
		}
	return 0;
}

static const struct {CKeyword keyword; CStorageType storage;} keyword_storage[] = {
{CKEYWORD_STATIC, CSTORAGE_STATIC},
{CKEYWORD_EXTERN, CSTORAGE_EXTERN},
{CKEYWORD_CONST, CTYPE_CONST},
{CKEYWORD_AUTO, CSTORAGE_AUTO},
{CKEYWORD_REGISTER, CSTORAGE_REGISTER},
{CKEYWORD_NONE, CSTORAGE_DEFAULT}};

static CKeyword CStorageType_keyword(CStorageType storage)
{
	size_t i;

	for (i = 0; keyword_storage[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_storage[i].storage == storage)
			return keyword_storage[i].keyword;
	return CKEYWORD_NONE;
}

const char* CStorageType_str(CStorageType storage)
{
	return CKeyword_str(CStorageType_keyword(storage));
}

static int get_storage(CKeyword keyword, CStorageType *storage)
{
	size_t i;

	for (i = 0; keyword_storage[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_storage[i].keyword == keyword) {
			*storage = keyword_storage[i].storage;
			return 1;
		}
	return 0;
}

static int poll_attributes(CScope *scope, StreamCToken *tokens, CTypeFlag *flags, CStorageType *pstorage)
{
	CKeyword keyword;
	CTypeFlag flag;
	CStorageType storage;
	CContext ctx;

	*flags = 0;
	if (pstorage != NULL)
		*pstorage = CSTORAGE_DEFAULT;
	while (CKeyword_poll(scope, tokens, &keyword, &ctx)) {
		if (get_flag(keyword, &flag)) {
			if (*flags & flag) {
				printf_error(ctx, "redeclaration of flag '%s'", CTypeFlag_str(flag));
				return 0;
			}
			*flags |= flag;
		}
		else if (get_storage(keyword, &storage)) {
			if (pstorage == NULL) {
				printf_error(ctx, "storage specifier '%s' in type definition",
				CStorageType_str(storage));
				return 0;
			}
			if (*pstorage != CSTORAGE_DEFAULT) {
				printf_error(ctx, "storage specifier redefinition: '%s' to '%s'",
				CStorageType_str(*pstorage), CStorageType_str(storage));
				return 0;
			}
			*pstorage = storage;
		} else {
			StreamCToken_back(tokens);
			break;
		}
	}
	return 1;
}

typedef enum {
	CPRIMITIVE_FLAG_NONE = 0,
	CPRIMITIVE_FLAG_UNSIGNED = 1,
	CPRIMITIVE_FLAG_SIGNED = 2,
	CPRIMITIVE_FLAG_CHAR = 4,
	CPRIMITIVE_FLAG_SHORT = 8,
	CPRIMITIVE_FLAG_INT = 16,
	CPRIMITIVE_FLAG_FLOAT = 32,
	CPRIMITIVE_FLAG_DOUBLE = 64,
	CPRIMITIVE_FLAG_VOID = 128
} CPrimitiveFlag;

static const struct {CKeyword keyword; CPrimitiveFlag flag;} keyword_primitive_flag[] = {
{CKEYWORD_UNSIGNED, CPRIMITIVE_FLAG_UNSIGNED},
{CKEYWORD_SIGNED, CPRIMITIVE_FLAG_SIGNED},
{CKEYWORD_CHAR, CPRIMITIVE_FLAG_CHAR},
{CKEYWORD_SHORT, CPRIMITIVE_FLAG_SHORT},
{CKEYWORD_INT, CPRIMITIVE_FLAG_INT},
{CKEYWORD_FLOAT, CPRIMITIVE_FLAG_FLOAT},
{CKEYWORD_DOUBLE, CPRIMITIVE_FLAG_DOUBLE},
{CKEYWORD_VOID, CPRIMITIVE_FLAG_VOID},
{CKEYWORD_NONE, CSTORAGE_DEFAULT}};

static CKeyword CPrimitiveFlag_keyword(CPrimitiveFlag flag)
{
	size_t i;

	for (i = 0; keyword_primitive_flag[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_primitive_flag[i].flag == flag)
			return keyword_primitive_flag[i].keyword;
	return CKEYWORD_NONE;
}

const char* CPrimitiveFlag_str(CPrimitiveFlag flag)
{
	return CKeyword_str(CPrimitiveFlag_keyword(flag));
}

static int get_primitive_flag(CKeyword keyword, CPrimitiveFlag *flag)
{
	size_t i;

	for (i = 0; keyword_primitive_flag[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_primitive_flag[i].keyword == keyword) {
			*flag = keyword_primitive_flag[i].flag;
			return 1;
		}
	return 0;
}

static void print_primitive(CPrimitiveFlag flags, size_t longCount)
{
	size_t count = longCount;
	size_t cur = 0;
	size_t i;

	for (i = 0; i < sizeof(flags) * 8; i++)
		if (flags & (1 << i))
			count++;
	for (i = 0; i < longCount; i++) {
		cur++;
		printf("long");
		if (cur < count)
			printf(" ");
	}
	for (i = 0; i < sizeof(flags) * 8; i++)
		if (flags & (1 << i)) {
			cur++;
			printf(CPrimitiveFlag_str(1 << i));
			if (cur < count)
				printf(" ");
		}
}

static int are_authorized_flags_only(CPrimitiveFlag flags, size_t longCount, CContext ctx, CPrimitiveFlag *auth, size_t maxLong)
{
	CPrimitiveFlag flag;
	int isAuth;
	size_t i;
	size_t j;

	if (longCount > maxLong) {
		printf_error_part(ctx, "incoherent type identifier 'long' for '");
		print_primitive(flags, longCount);
		printf("'\n\n");
		return 0;
	}
	for (j = 0; j < sizeof(flags) * 8; j++) {
		flag = 1 << j;
		if (!(flags & flag))
			continue;
		isAuth = 0;
		for (i = 0; auth[i] != CPRIMITIVE_FLAG_NONE; i++) {
			if (flag == auth[i]) {
				isAuth = 1;
				break;
			}
		}
		if (!isAuth) {
			printf_error_part(ctx, "incoherent type identifier '%s' for '", CPrimitiveFlag_str(flag));
			print_primitive(flags ^ flag, longCount);
			printf("'\n\n");
			return 0;
		}
	}
	return 1;
}

static int are_flags_coherent(CPrimitiveFlag flags, size_t longCount, CContext ctx)
{
	CPrimitiveFlag a_char[] = {CPRIMITIVE_FLAG_SIGNED, CPRIMITIVE_FLAG_UNSIGNED, CPRIMITIVE_FLAG_INT, CPRIMITIVE_FLAG_CHAR, 0};
	CPrimitiveFlag a_short[] = {CPRIMITIVE_FLAG_SIGNED, CPRIMITIVE_FLAG_UNSIGNED, CPRIMITIVE_FLAG_INT, CPRIMITIVE_FLAG_SHORT, 0};
	CPrimitiveFlag a_int[] = {CPRIMITIVE_FLAG_SIGNED, CPRIMITIVE_FLAG_UNSIGNED, CPRIMITIVE_FLAG_INT, 0};
	CPrimitiveFlag a_long[] = {CPRIMITIVE_FLAG_SIGNED, CPRIMITIVE_FLAG_UNSIGNED, CPRIMITIVE_FLAG_INT, 0};
	CPrimitiveFlag a_float[] = {CPRIMITIVE_FLAG_FLOAT, 0};
	CPrimitiveFlag a_double[] = {CPRIMITIVE_FLAG_DOUBLE, 0};
	CPrimitiveFlag a_void[] = {CPRIMITIVE_FLAG_VOID, 0};

	if (flags & CPRIMITIVE_FLAG_UNSIGNED)
		if (flags & CPRIMITIVE_FLAG_SIGNED) {
			printf_error(ctx, "have 'unsigned' and 'signed' type identifiers");
			return 0;
		}
	if (flags & CPRIMITIVE_FLAG_SIGNED)
		if (flags & CPRIMITIVE_FLAG_UNSIGNED) {
			printf_error(ctx, "have 'signed' and 'unsigned' type identifiers");
			return 0;
		}

	if (flags & CPRIMITIVE_FLAG_CHAR)
		if (!are_authorized_flags_only(flags, longCount, ctx, a_char, 0))
			return 0;
	if (flags & CPRIMITIVE_FLAG_SHORT)
		if (!are_authorized_flags_only(flags, longCount, ctx, a_short, 0))
			return 0;
	if (flags & CPRIMITIVE_FLAG_INT)
		if (!are_authorized_flags_only(flags, longCount, ctx, a_int, 0))
			return 0;
	if (longCount > 0) {
		if (flags & CPRIMITIVE_FLAG_DOUBLE) {
			if (!are_authorized_flags_only(flags, longCount, ctx, a_double, 1))
				return 0;
		} else if (!are_authorized_flags_only(flags, longCount, ctx, a_long, 2))
			return 0;
	}
	if (flags & CPRIMITIVE_FLAG_FLOAT)
		if (!are_authorized_flags_only(flags, longCount, ctx, a_float, 0))
			return 0;
	if (flags & CPRIMITIVE_FLAG_DOUBLE)
		if (!are_authorized_flags_only(flags, longCount, ctx, a_double, 1))
			return 0;
	if (flags & CPRIMITIVE_FLAG_VOID)
		if (!are_authorized_flags_only(flags, longCount, ctx, a_void, 0))
			return 0;
	return 1;
}

static CPrimitiveType get_int_primitive_type(CPrimitiveFlag flags)
{
	if (flags & CPRIMITIVE_FLAG_UNSIGNED)
		return CPRIMITIVE_UINT;
	else
		return CPRIMITIVE_SINT;
}

static int poll_primitive(CScope *scope, StreamCToken *tokens, CPrimitive *pres)
{
	CKeyword keyword;
	CPrimitiveFlag flags = 0;
	size_t longCount = 0;
	CPrimitiveFlag flag;
	CContext ctx;

	while (CKeyword_poll(scope, tokens, &keyword, &ctx)) {
		if (keyword == CKEYWORD_LONG) {
			longCount++;
			if (!are_flags_coherent(flags, longCount, ctx))
				return 0;
		}
		else if (get_primitive_flag(keyword, &flag)) {
			if (flags & flag) {
				printf_error(ctx, "redeclaration of type identifier '%s'", CPrimitiveFlag_str(flag));
				return 0;
			}
			flags |= flag;
			if (!are_flags_coherent(flags, longCount, ctx))
				return 0;
		} else {
			StreamCToken_back(tokens);
			break;
		}
	}
	if (flags & CPRIMITIVE_FLAG_CHAR) {
		pres->type = get_int_primitive_type(flags);
		pres->data = (void*)1;
		return 1;
	} else if (flags & CPRIMITIVE_FLAG_SHORT) {
		pres->type = get_int_primitive_type(flags);
		pres->data = (void*)2;
		return 1;
	} else if (flags & CPRIMITIVE_FLAG_INT) {
		pres->type = get_int_primitive_type(flags);
		pres->data = (void*)4;
		return 1;
	} else if (longCount == 1) {
		pres->type = get_int_primitive_type(flags);
		pres->data = (void*)4;
		return 1;
	} else if (flags & CPRIMITIVE_FLAG_FLOAT) {
		pres->type = CPRIMITIVE_FLOAT;
		pres->data = (void*)4;
		return 1;
	} else if (flags & CPRIMITIVE_FLAG_DOUBLE) {
		pres->type = CPRIMITIVE_FLOAT;
		pres->data = (void*)8;
		return 1;
	} else if (flags & CPRIMITIVE_FLAG_VOID) {
		pres->type = CPRIMITIVE_VOID;
		pres->data = NULL;
		return 1;
	} else if ((flags & CPRIMITIVE_FLAG_SIGNED) || (flags & CPRIMITIVE_FLAG_UNSIGNED)) {
		pres->type = get_int_primitive_type(flags);;
		pres->data = (void*)4;
		return 1;
	} else {
		printf_error_part(StreamCToken_atCtx(tokens), "unsupported type '");
		print_primitive(flags, longCount);
		printf("'\n\n");
		return 0;
	}
}

static size_t poll_reference_level(StreamCToken *tokens)
{
	CToken cur;
	size_t res = 0;

	while (StreamCToken_at(tokens, &cur)) {
		if (streq(cur.str, "*")) {
			res++;
			StreamCToken_forward(tokens);
		} else
			break;
	}
	return res;
}

static int poll_reference(StreamCToken *tokens, char **pname, CReference *acc)
{
	CToken cur;
	char *err_msg = "expected name for declaration";
	//char *err_msg_fun = "expected name for function declaration";

	acc->level += poll_reference_level(tokens);
	if (!StreamCToken_poll(tokens, &cur)) {
		printf_error(StreamCToken_lastCtx(tokens), err_msg);
		return 0;
	}
	if (str_is_identifier(cur.str))
		*pname = cur.str;
	else
		StreamCToken_back(tokens);
	return 1;
}

static CFunction* CFunction_alloc(CFunction base)
{
	CFunction *res = (CFunction*)malloc(sizeof(CFunction));

	*res = base;
	return res;
}

static void CFunction_addArg(CFunction *func, CType *to_add)
{
	size_t cur = func->argCount++;

	func->arg = (CType**)realloc(func->arg, func->argCount * sizeof(CType*));
	func->arg[cur] = to_add;
}

void CFunction_destroy(CFunction *func)
{
	size_t i;

	CType_destroy(func->returnType);
	for (i = 0; i < func->argCount; i++)
		CType_destroy(func->arg[i]);
	free(func->arg);
	free(func);
}

static CType* CType_initFunction(CType *returnType, CReference funReference)
{
	CType res = CType_default();
	CFunction fun;

	fun.returnType = returnType;
	fun.argCount = 0;
	fun.arg = NULL;
	res.ref = funReference;
	res.primitive.type = CPRIMITIVE_FUNCTION;
	res.primitive.data = CFunction_alloc(fun);
	return CType_alloc(res);
}

static int poll_function(CScope *scope, StreamCToken *tokens, int hasPolled, CType *pres, VecStr *pargsName)
{
	CType *type;
	VecStr args = VecStr_init();
	CToken cur;
	CContext ctx;
	int forceContinue = 0;
	char *name;

	if (!hasPolled)
		if (!StreamCToken_pollLpar(tokens, &ctx)) {
			printf_error(ctx, "missing '(' for declaring function arguments");
			return 0;
		}
	while ((!StreamCToken_pollRpar(tokens, &ctx)) || forceContinue) {
		if (!CType_parse(scope, tokens, &name, &type, NULL, NULL)) {
			VecStr_destroy(args);
			return 0;
		}
		CFunction_addArg(pres->primitive.data, type);
		VecStr_add(&args, strdup(name));
		forceContinue = StreamCToken_pollStr(tokens, ",", NULL);
	}
	if (pargsName != NULL)
		*pargsName = args;
	else
		VecStr_destroy(args);
	return 1;
}

static CVariable CVariable_default(void)
{
	CVariable res;

	res.name = NULL;
	res.address = 0;
	res.storage = CSTORAGE_DEFAULT;
	res.type = NULL;
	return res;
}

static CVariable* CVariable_alloc(CVariable base)
{
	CVariable *res = (CVariable*)malloc(sizeof(CVariable));

	*res = base;
	return res;
}

int CVariable_parse(CScope *scope, StreamCToken *tokens, CVariable **pres, VecStr *pargs)
{
	CVariable *res = CVariable_alloc(CVariable_default());
	char *name;

	if (!CType_parse(scope, tokens, &name, &res->type, &res->storage, pargs)) {
		CVariable_destroy(res);
		return 0;
	}
	if (name != NULL)
		res->name = strdup(name);
	else
		res->name = NULL;
	*pres = res;
	return 1;
}

void CVariable_destroy(CVariable *variable)
{
	free(variable->name);
	CType_destroy(variable->type);
	free(variable);
}

CType* CType_alloc(CType base)
{
	CType *res = (CType*)malloc(sizeof(CType));

	*res = base;
	return res;
}

int CType_parse(CScope *scope, StreamCToken *tokens, char **pname, CType **pres, CStorageType *pstorage, VecStr *pargsName)
{
	CType *res = CType_alloc(CType_default());
	int isFun = 0;
	int hasPolled;
	CReference funRef = CReference_default();
	CContext ctx;

	*pname = NULL;
	if (!poll_attributes(scope, tokens, &res->flags, pstorage))
		goto CType_parse_error;
	if (!poll_primitive(scope, tokens, &res->primitive))
		goto CType_parse_error;
	if (!poll_reference(tokens, pname, &res->ref))
		goto CType_parse_error;
	isFun = StreamCToken_pollLpar(tokens, NULL);
	if (isFun) {
		if (!poll_reference(tokens, pname, &funRef))
			goto CType_parse_error;
		if (!StreamCToken_pollRpar(tokens, &ctx)) {
			printf_error(ctx, "missing ')' for closing function name");
			goto CType_parse_error;
		}
		res = CType_initFunction(res, funRef);
	}
	hasPolled = StreamCToken_pollLpar(tokens, NULL);
	if (hasPolled || isFun)
		if (!poll_function(scope, tokens, hasPolled, res, pargsName))
			goto CType_parse_error;
	if (*pname == NULL)
		if (!poll_reference(tokens, pname, &res->ref))
			goto CType_parse_error;
	*pres = res;
	return 1;

	CType_parse_error:
	CType_destroy(res);
	return 0;
}

static void print_tabs(size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
		printf("  ");
}

static void CType_print_actual(CType *type, size_t depth);

static void CVariable_print(CVariable *var, size_t depth)
{
	print_tabs(depth);
	printf("name: '%s'\n", var->name != NULL ? var->name : "(null)");
	print_tabs(depth);
	printf("address: %u\n", var->address);
	print_tabs(depth);
	printf("storage: %u\n", var->storage);
	CType_print_actual(var->type, depth + 1);
}

static void CFunction_print(CFunction *func, size_t depth)
{
	size_t i;

	print_tabs(depth);
	printf("return:\n");
	CType_print_actual(func->returnType, depth + 1);
	for (i = 0; i < func->argCount; i++) {
		print_tabs(depth);
		printf("arg #%u:\n", i);
		CType_print_actual(func->arg[i], depth + 1);
	}
}

static void CType_print_actual(CType *type, size_t depth)
{
	size_t i;

	print_tabs(depth);
	printf("flags: %d\n", type->flags);
	print_tabs(depth);
	printf("refLevel: %u\n", type->ref.level);
	print_tabs(depth);
	printf("array (%u): ", type->ref.arrayCount);
	for (i = 0; i < type->ref.arrayCount; i++) {
		if (type->ref.array[i].isUndef)
			printf("[]");
		else
			printf("[%u]", type->ref.array[i].size);
	}
	printf("\n");
	print_tabs(depth);
	printf("Primitive: ");
	switch (type->primitive.type) {
	case CPRIMITIVE_VOID:
		printf("void");
		break;
	case CPRIMITIVE_UINT:
		printf("uint: %u", type->primitive.data);
		break;
	case CPRIMITIVE_SINT:
		printf("sint: %u", type->primitive.data);
		break;
	case CPRIMITIVE_FLOAT:
		printf("float: %u", type->primitive.data);
		break;
	case CPRIMITIVE_FUNCTION:
		printf("function:\n", type->primitive.data);
		CFunction_print(type->primitive.data, depth + 1);
		break;
	}
	printf("\n");
}

static void CType_print(CType *type)
{
	terminal_flush();
	CType_print_actual(type, 0);
	terminal_show();
}

void CType_destroy(CType *type)
{
	if (type == NULL)
		return;
	CReference_destroy(type->ref);
	CPrimitive_destroy(type->primitive);
	free(type);
}
