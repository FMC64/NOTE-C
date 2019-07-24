
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
	res.isDataNamed = 0;
	return res;
}

void CPrimitive_destroy(CPrimitive primitive)
{
	if (primitive.isDataNamed)
		return;
	switch (primitive.type) {
	case CPRIMITIVE_FUNCTION:
		CFunction_destroy(primitive.data);
		break;
	}
}

static CType CType_default(void)
{
	CType res;

	res.flags = CTYPE_NONE;
	res.isTypeNamed = 0;
	res.referenceLevel = 0;
	res.full = NULL;
	return res;
}

CType CType_fromFull(CTypeFull *full)
{
	CType res = CType_default();

	res.full = full;
	return res;
}

CPrimitiveType CType_primitiveType(CType type)
{
	return type.full->primitive.type;
}

void* CType_primitiveData(CType type)
{
	return type.full->primitive.data;
}

size_t CType_referenceLevel(CType type)
{
	return type.referenceLevel + type.full->ref.level;
}

void CType_destroy(CType type)
{
	if (!type.isTypeNamed)
		CTypeFull_destroy(type.full);
}

CTypeFull CTypeFull_default(void)
{
	CTypeFull res;

	res.flags = 0;
	res.ref = CReference_default();
	res.primitive = CPrimitive_default();
	return res;
}

CTypeFull* CTypeFull_createPrimitive(CPrimitiveType type, size_t bits)
{
	CTypeFull res = CTypeFull_default();

	res.primitive.type = type;
	res.primitive.data = (void*)bits;
	return CTypeFull_alloc(res);
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

static int poll_attributes(CScope *scope, CStream *tokens, CTypeFlag *flags, CStorageType *pstorage)
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
			CStream_back(tokens);
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

static int poll_primitive(CScope *scope, CStream *tokens, CPrimitive *pres)
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
			CStream_back(tokens);
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
		printf_error_part(CStream_atCtx(tokens), "unsupported type '");
		print_primitive(flags, longCount);
		printf("'\n\n");
		return 0;
	}
}

static size_t poll_reference_level(CStream *tokens)
{
	CToken cur;
	size_t res = 0;

	while (CStream_at(tokens, &cur)) {
		if (streq(cur.str, "*")) {
			res++;
			CStream_forward(tokens);
		} else
			break;
	}
	return res;
}

static int poll_reference(CStream *tokens, char **pname, CReference *acc)
{
	CToken cur;
	char *err_msg = "expected name for declaration";
	//char *err_msg_fun = "expected name for function declaration";

	if (pname != NULL)
		*pname = NULL;
	acc->level += poll_reference_level(tokens);
	if (!CStream_poll(tokens, &cur))
		return;
	if (pname != NULL) {
		if (str_is_identifier(cur.str))
			*pname = cur.str;
		else
			CStream_back(tokens);
	} else
		CStream_back(tokens);
	return 1;
}

static CFunction* CFunction_alloc(CFunction base)
{
	CFunction *res = (CFunction*)malloc(sizeof(CFunction));

	*res = base;
	return res;
}

static void CFunction_addArg(CFunction *func, CType to_add)
{
	size_t cur = func->argCount++;

	func->arg = (CType*)realloc(func->arg, func->argCount * sizeof(CType));
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

static CTypeFull* CTypeFull_initFunction(CScope *scope, CTypeFull *returnType, CReference funReference)
{
	CTypeFull res = CTypeFull_default();
	CFunction fun;

	fun.returnType = CType_fromFull(returnType);
	CType_shrink(scope, &fun.returnType);
	fun.argCount = 0;
	fun.arg = NULL;
	res.ref = funReference;
	res.primitive.type = CPRIMITIVE_FUNCTION;
	res.primitive.data = CFunction_alloc(fun);
	return CTypeFull_alloc(res);
}

static int poll_function(CScope *scope, CStream *tokens, int hasPolled, CTypeFull *pres, VecStr *pargsName)
{
	CType type;
	VecStr args = VecStr_init();
	CToken cur;
	CContext ctx;
	int forceContinue = 0;
	char *name;

	if (!hasPolled)
		if (!CStream_pollLpar(tokens, &ctx)) {
			printf_error(ctx, "missing '(' for declaring function arguments");
			return 0;
		}
	while ((!CStream_pollRpar(tokens, &ctx)) || forceContinue) {
		if (!CType_parseFull(scope, tokens, &name, &type, NULL, NULL)) {
			VecStr_destroy(args);
			return 0;
		}
		if (CType_primitiveType(type) != CPRIMITIVE_VOID) {
			CFunction_addArg(pres->primitive.data, type);
			VecStr_add(&args, name != NULL ? strdup(name) : name);
			forceContinue = CStream_pollStr(tokens, ",", NULL);
		}
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
	res.type = CType_default();
	return res;
}

static CVariable* CVariable_alloc(CVariable base)
{
	CVariable *res = (CVariable*)malloc(sizeof(CVariable));

	*res = base;
	return res;
}

int CVariable_parse(CScope *scope, CStream *tokens, CVariable **pres, VecStr *pargs)
{
	CVariable *res = CVariable_alloc(CVariable_default());
	char *name;

	if (!CTypeFull_parse(scope, tokens, &name, &res->type.full, &res->storage, pargs)) {
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

CTypeFull* CTypeFull_alloc(CTypeFull base)
{
	CTypeFull *res = (CTypeFull*)malloc(sizeof(CTypeFull));

	*res = base;
	return res;
}

int CTypeFull_parse(CScope *scope, CStream *tokens, char **pname, CTypeFull **pres, CStorageType *pstorage, VecStr *pargsName)
{
	CTypeFull *res = CTypeFull_alloc(CTypeFull_default());
	int isFun = 0;
	int hasPolled;
	CReference funRef = CReference_default();
	CContext ctx;

	*pname = NULL;
	if (!poll_attributes(scope, tokens, &res->flags, pstorage))
		goto CTypeFull_parse_error;
	if (!poll_primitive(scope, tokens, &res->primitive))
		goto CTypeFull_parse_error;
	if (!poll_reference(tokens, NULL, &res->ref))
		goto CTypeFull_parse_error;
	isFun = CStream_pollLpar(tokens, NULL);
	if (!poll_reference(tokens, pname, &funRef))
		goto CTypeFull_parse_error;
	if (isFun)
		if (!CStream_pollRpar(tokens, &ctx)) {
			printf_error(ctx, "missing ')' for closing function name");
			goto CTypeFull_parse_error;
		}
	hasPolled = CStream_pollLpar(tokens, NULL);
	if (hasPolled || isFun) {
		res = CTypeFull_initFunction(scope, res, funRef);
		if (!poll_function(scope, tokens, hasPolled, res, pargsName))
			goto CTypeFull_parse_error;
	}
	if (*pname == NULL)
		if (!poll_reference(tokens, pname, &res->ref))
			goto CTypeFull_parse_error;
	*pres = res;
	return 1;

	CTypeFull_parse_error:
	CTypeFull_destroy(res);
	return 0;
}

static int VecCTypeFull_searchPrimitive(VecCTypeFull *vec, size_t bits, CTypeFull **pres)
{
	size_t i;

	for (i = 0; i < vec->count; i++)
		if ((size_t)vec->type[i]->primitive.data == bits) {
			*pres = vec->type[i];
			return 1;
		}
	return 0;
}

static void finish_shrink(CTypeFull *from, CTypeFull *to, CType *shrinked)
{
	shrinked->flags = from->flags;
	shrinked->referenceLevel = from->ref.level - to->ref.level;
	shrinked->isTypeNamed = 1;
	shrinked->full = to;
	CTypeFull_destroy(from);
}

void CType_shrink(CScope *scope, CType *to_shrink)
{
	CTypeFull *full = to_shrink->full;
	CTypeFull *found;

	if (to_shrink->isTypeNamed)
		return;
	if (full->ref.arrayCount > 0)
		return;
	switch (full->primitive.type) {
	case CPRIMITIVE_VOID:
		finish_shrink(full, (CTypeFull*)scope->cachedTypes.t_void, to_shrink);
		return;
	case CPRIMITIVE_UINT:
		if (!VecCTypeFull_searchPrimitive(&scope->cachedTypes.t_uint, (size_t)full->primitive.data, &found))
			return;
		finish_shrink(full, found, to_shrink);
		return;
	case CPRIMITIVE_SINT:
		if (!VecCTypeFull_searchPrimitive(&scope->cachedTypes.t_sint, (size_t)full->primitive.data, &found))
			return;
		finish_shrink(full, found, to_shrink);
		return;
	case CPRIMITIVE_FLOAT:
		if (!VecCTypeFull_searchPrimitive(&scope->cachedTypes.t_float, (size_t)full->primitive.data, &found))
			return;
		finish_shrink(full, found, to_shrink);
		return;
	}
}

int CType_parseFull(CScope *scope, CStream *tokens, char **pname, CType *pres, CStorageType *pstorage, VecStr *pargsName)
{
	CType res = CType_default();

	if (!CTypeFull_parse(scope, tokens, pname, &res.full, pstorage, pargsName))
		return 0;
	CType_shrink(scope, &res);
	*pres = res;
	return 1;
}

int CType_parse(CScope *scope, CStream *tokens, CType *pres)
{
	char *name;

	return CType_parseFull(scope, tokens, &name, pres, NULL, NULL);
}

static void print_tabs(size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
		printf("  ");
}

static void CType_print_tree_actual(CType type, size_t depth);

static void CVariable_print_tree(CVariable *var, size_t depth)
{
	print_tabs(depth);
	printf("name: '%s'\n", var->name != NULL ? var->name : "(null)");
	print_tabs(depth);
	printf("address: %u\n", var->address);
	print_tabs(depth);
	printf("storage: %u\n", var->storage);
	print_tabs(depth);
	printf("type:\n");
	CType_print_tree_actual(var->type, depth + 1);
}

static void CFunction_print_tree(CFunction *func, size_t depth)
{
	size_t i;

	print_tabs(depth);
	printf("return:\n");
	CType_print_tree_actual(func->returnType, depth + 1);
	print_tabs(depth);
	printf("%u args:\n", func->argCount);
	for (i = 0; i < func->argCount; i++) {
		print_tabs(depth);
		printf("arg #%u:\n", i);
		CType_print_tree_actual(func->arg[i], depth + 1);
	}
}

static void CTypeFull_print_tree(CTypeFull *type, size_t depth)
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
		CFunction_print_tree(type->primitive.data, depth + 1);
		break;
	}
	printf("\n");
}

static void CType_print_tree_actual(CType type, size_t depth)
{
	print_tabs(depth);
	printf("flags: %u\n", type.flags);
	print_tabs(depth);
	printf("isTypeNamed: %u\n", type.isTypeNamed);
	print_tabs(depth);
	printf("refLevel: %u\n", type.referenceLevel);
	CTypeFull_print_tree(type.full, depth + 1);
}

void CType_print_tree(CType type)
{
	terminal_flush();
	CType_print_tree_actual(type, 0);
	terminal_show();
}

static void CTypeFlags_print(CTypeFlag flags)
{
	size_t i;
	CTypeFlag cur;

	for (i = 0; i < sizeof(flags) * 8; i++) {
		cur = 1 << i;
		if (flags & cur)
			printf("%s ", CTypeFlag_str(cur));
	}
}

static void print_primitive_num(CPrimitiveType type, size_t bytes)
{
	if (type == CPRIMITIVE_VOID)
		printf("void");
	if (type == CPRIMITIVE_UINT)
		printf("unsigned ");
	else if (type == CPRIMITIVE_UINT)
		printf("signed ");
	if ((type == CPRIMITIVE_UINT) || (type == CPRIMITIVE_SINT))
		switch (bytes) {
		case 1:
			printf("char");
			break;
		case 2:
			printf("short");
			break;
		case 4:
			printf("int");
			break;
		case 8:
			printf("long long");
			break;
		}
	if (type == CPRIMITIVE_FLOAT)
		switch (bytes) {
		case 4:
			printf("float");
			break;
		case 8:
			printf("double");
			break;
		}
}

static void print_reference(CType type)
{
	size_t level = CType_referenceLevel(type);
	size_t i;

	for (i = 0; i < level; i++)
		printf("*");
	for (i = 0; i < type.full->ref.arrayCount; i++) {
		printf("[");
		if (!type.full->ref.array[i].isUndef)
			printf("%u", type.full->ref.array[i].size);
		printf("]");
	}
}

static void print_function(CFunction *func, CType base)
{
	size_t i;

	CType_print(func->returnType);
	printf(" (");
	print_reference(base);
	printf(")(");
	for (i = 0; i < func->argCount; i++) {
		CType_print(func->arg[i]);
		if (i < (func->argCount - 1))
			printf(", ");
	}
	printf(")");
}

void CType_print(CType type)
{
	CTypeFlags_print(type.flags);
	switch (CType_primitiveType(type)) {
	case CPRIMITIVE_FUNCTION:
		print_function(CType_primitiveData(type), type);
		break;
	case CPRIMIVITE_STRUCT:
		printf("%s %s", CStruct_type(CType_primitiveData(type)), CStruct_name(CType_primitiveData(type)));
		print_reference(type);
		break;
	default:
		print_primitive_num(CType_primitiveType(type), (size_t)CType_primitiveData(type));
		print_reference(type);
		break;
	}
}

void CTypeFull_destroy(CTypeFull *type)
{
	if (type == NULL)
		return;
	CReference_destroy(type->ref);
	CPrimitive_destroy(type->primitive);
	free(type);
}

char* CStruct_type(CStruct *str)
{
	if (str->isUnion)
		return "union";
	else
		return "struct";
}

char* CStruct_name(CStruct *str)
{
	if (str->name != NULL)
		return str->name;
	else
		return "{unnamed}";
}
