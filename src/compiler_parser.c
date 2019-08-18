
#include "headers.h"

static VecCTypeFull VecCTypeFull_init(void)
{
	VecCTypeFull res;

	res.count = 0;
	res.type = NULL;
	return res;
}

static void VecCTypeFull_add(VecCTypeFull *vec, CTypeFull *to_add)
{
	size_t cur = vec->count++;

	vec->type = (CTypeFull**)realloc(vec->type, vec->count * sizeof(CTypeFull*));
	vec->type[cur] = to_add;
}

static void VecCTypeFull_destroy(VecCTypeFull vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		CTypeFull_destroy(vec.type[i]);
	free(vec.type);
}

static CTypeFullCached CTypeFullCached_create(void)
{
	CTypeFullCached res;
	size_t int_bytes[3] = {1, 2, 4};
	size_t float_bytes[2] = {4, 8};
	size_t i;

	res.t_void = CTypeFull_createPrimitive(CPRIMITIVE_VOID, 0);
	res.t_uint = VecCTypeFull_init();
	res.t_sint = VecCTypeFull_init();
	for (i = 0; i < 3; i++) {
		VecCTypeFull_add(&res.t_uint, CTypeFull_createPrimitive(CPRIMITIVE_UINT, int_bytes[i]));
		VecCTypeFull_add(&res.t_sint, CTypeFull_createPrimitive(CPRIMITIVE_SINT, int_bytes[i]));
	}
	res.t_float = VecCTypeFull_init();
	for (i = 0; i < 2; i++)
		VecCTypeFull_add(&res.t_float, CTypeFull_createPrimitive(CPRIMITIVE_FLOAT, float_bytes[i]));
	return res;
}

static void CTypeFullCached_destroy(CTypeFullCached cached)
{
	CTypeFull_destroy(cached.t_void);
	VecCTypeFull_destroy(cached.t_uint);
	VecCTypeFull_destroy(cached.t_sint);
	VecCTypeFull_destroy(cached.t_float);
}

static const struct {const char *key; CKeyword keyword;} str_keyword[] = {
{"auto", CKEYWORD_AUTO},
{"break", CKEYWORD_BREAK},
{"case", CKEYWORD_CASE},
{"char", CKEYWORD_CHAR},
{"const", CKEYWORD_CONST},
{"continue", CKEYWORD_CONTINUE},
{"default", CKEYWORD_DEFAULT},
{"do", CKEYWORD_DO},
{"double", CKEYWORD_DOUBLE},
{"else", CKEYWORD_ELSE},
{"enum", CKEYWORD_ENUM},
{"extern", CKEYWORD_EXTERN},
{"float", CKEYWORD_FLOAT},
{"for", CKEYWORD_FOR},
{"goto", CKEYWORD_GOTO},
{"if", CKEYWORD_IF},
{"inline", CKEYWORD_INLINE},
{"int", CKEYWORD_INT},
{"long", CKEYWORD_LONG},
{"register", CKEYWORD_REGISTER},
{"return", CKEYWORD_RETURN},
{"short", CKEYWORD_SHORT},
{"signed", CKEYWORD_SIGNED},
{"sizeof", CKEYWORD_SIZEOF},
{"static", CKEYWORD_STATIC},
{"struct", CKEYWORD_STRUCT},
{"switch", CKEYWORD_SWITCH},
{"typedef", CKEYWORD_TYPEDEF},
{"union", CKEYWORD_UNION},
{"unsigned", CKEYWORD_UNSIGNED},
{"void", CKEYWORD_VOID},
{"volatile", CKEYWORD_VOLATILE},
{"while", CKEYWORD_WHILE},
{NULL, CKEYWORD_NONE}};

static StrSonic keywords;

int CScope_keywords_init(void)
{
	size_t i;

	keywords = StrSonic_init(&StrSonic_CSymbol_destroy);
	for (i = 0; str_keyword[i].key != NULL; i++)
		if (!StrSonic_addCSymbol(&keywords, str_keyword[i].key, CKeyword_create(str_keyword[i].keyword)))
			return 0;
	return 1;
}

void CScope_keywords_quit(void)
{
	StrSonic_destroy(&keywords);
}

int CScope_create(const char *filepath, CScope **pres)
{
	CScope *res = (CScope*)malloc(sizeof(CScope));

	if (!CStream_create(filepath, &res->stream)) {
		free(res);
		return 0;
	}
	res->blockCount = 0;
	res->block = NULL;
	res->cachedTypes = CTypeFullCached_create();
	CScope_addBlock(res, CBlock_default());
	*pres = res;
	return 1;
}

static int CScope_isVoid(CScope *scope, CContext ctx)
{
	if (scope->blockCount == 0) {
		printf_error(ctx, "noscope");
		return 1;
	}
	return 0;
}

// Make sure scope is not empty before calling
static CBlock* CScope_lastBlock(CScope *scope)
{
	return &scope->block[scope->blockCount - 1];
}

void CScope_addBlock(CScope *scope, CBlock to_add)
{
	size_t cur = scope->blockCount++;

	scope->block = (CBlock*)realloc(scope->block, scope->blockCount * sizeof(CBlock));
	scope->block[cur] = to_add;
}

int CScope_removeBlock(CScope *scope, CContext ctx)
{
	if (CScope_isVoid(scope, ctx))
		return 0;
	CBlock_destroy(scope->block[--scope->blockCount]);
	if (scope->blockCount == 0) {
		printf_error(ctx, "you just destroyed file scope. congratulations.");
		return 0;
	}
	return 1;
}

int CScope_addSymbol(CScope *scope, const char *key, CSymbol to_add, CContext ctx)
{
	if (CScope_isVoid(scope, ctx))
		return 0;
	if (!StrSonic_add(&CScope_lastBlock(scope)->symbols, key, to_add.type, to_add.data)) {
		printf_error(ctx, "redefinition of symbol '%s'", key);
		return 0;
	}
	return 1;
}

int CScope_resolve(CScope *scope, const char *key, CSymbol *pres)
{
	size_t i;
	size_t i_rev;

	if (StrSonic_resolveCSymbol(&keywords, key, pres))
		return 1;
	for (i = 0; i < scope->blockCount; i++) {
		i_rev = scope->blockCount - 1 - i;
		if (StrSonic_resolveCSymbol(&scope->block[i_rev].symbols, key, pres))
			return 1;
	}
	return 0;
}

void CScope_destroy(CScope *scope)
{
	size_t i;

	for (i = 0; i < scope->blockCount; i++)
		CBlock_destroy(scope->block[i]);
	free(scope->block);
	CTypeFullCached_destroy(scope->cachedTypes);
	CStream_destroy(scope->stream);
	free(scope);
}

CBlock CBlock_default(void)
{
	CBlock res;

	res.symbols = StrSonic_init(&StrSonic_CSymbol_destroy);
	return res;
}

void CBlock_destroy(CBlock block)
{
	StrSonic_destroy(&block.symbols);
}

CSymbol CSymbol_init(CSymbolType type, void *data)
{
	CSymbol res;

	res.type = type;
	res.data = data;
	return res;
}

static int CSymbol_eq_adv(CSymbol a, CSymbol b, int do_strict)
{
	if (a.type != b.type)
		return 0;
	switch (a.type) {
	case CSYMBOL_TYPE:
		return CType_eq_adv(*(CType*)a.data, *(CType*)b.data, do_strict);
	case CSYMBOL_STRUCT:
		return CStruct_eq(a.data, b.data);
	case CSYMBOL_FUNCTION:
		return CFunction_eq(a.data, b.data);
	default:
		return a.data != b.data;
	}
}

int CSymbol_eq(CSymbol a, CSymbol b)
{
	return CSymbol_eq_adv(a, b, 0);
}

int CSymbol_eq_strict(CSymbol a, CSymbol b)
{
	return CSymbol_eq_adv(a, b, 1);
}

void CSymbol_print(CSymbol sym)
{
	switch (sym.type) {
	case CSYMBOL_TYPE:
		printf("type: ");
		CType_print(*(CType*)sym.data);
		return;
	case CSYMBOL_STRUCT:
		CStruct_print(sym.data);
		return;
	case CSYMBOL_FUNCTION:
		printf("function: ");
		CFunction_print_name_noref(sym.data, NULL);
		return;
	default:
		printf("symbol type %d, data %u", sym.type, sym.data);
		return;
	}
}

void CSymbol_destroy(CSymbol symbol)
{
	switch (symbol.type) {
	case CSYMBOL_KEYWORD:
		CKeyword_destroy(symbol.data);
		return;
	case CSYMBOL_TYPE:
		CType_destroy(*(CType*)symbol.data);
		free(symbol.data);
		return;
	case CSYMBOL_STRUCT:
		CStruct_destroy(symbol.data);
		return;
	case CSYMBOL_FUNCTION:
		CFunction_destroy(symbol.data);
		return;
	}
}

void StrSonic_CSymbol_destroy(unsigned char type, void *data)
{
	CSymbol_destroy(CSymbol_init(type, data));
}

CSymbol CKeyword_create(CKeyword keyword)
{
	return CSymbol_init(CSYMBOL_KEYWORD, (void*)keyword);
}

const char* CKeyword_str(CKeyword keyword)
{
	size_t i;

	for (i = 0; str_keyword[i].key != NULL; i++)
		if (str_keyword[i].keyword == keyword)
			return str_keyword[i].key;
	return "undefined";
}

void CKeyword_destroy(void *data)
{
}

CParser CParser_init(char *source_path)
{
	CParser res;

	res.buf = CBuf_init(source_path);
	return res;
}

static void print_error_unexp_token_at(CScope *scope)
{
	const char *str = "{undefined}";
	CToken cur;

	if (CStream_at(scope->stream, &cur)) {
		printf_error_part(cur.ctx, "unexpected token: ");
		CToken_print(cur);
		printf("\n\n");
	} else
		printf_error(CStream_atCtx(scope->stream), "unexpected token: %s", str);
}

int add_type(CScope *scope, const char *name, CType to_add, CContext ctx)
{
	CSymbol entry = CSymbol_init(CSYMBOL_TYPE, CType_alloc(to_add));
	CSymbol had;

	if (name == NULL) {
		printf_error(ctx, "no name for type declaration");
		free(entry.data);
		return 0;
	}
	if (CScope_resolve(scope, name, &had)) {
		if (!CSymbol_eq(entry, had)) {
			printf_error_symbol_redef(name, had, entry, ctx);
			free(entry.data);
			return 0;
		}
		CSymbol_destroy(entry);
		return 1;
	}
	if (!CScope_addSymbol(scope, name, entry, ctx)) {
		free(entry.data);
		return 0;
	}
	return 1;
}

static int skip_block(CScope *scope)
{
	CToken cur;
	size_t level = 0;

	while (CStream_poll(scope->stream, &cur)) {
		if (CToken_streq(cur, "{"))
			level++;
		if (CToken_streq(cur, "}")) {
			level--;
			if (level == 0)
				break;
		}
		if (CToken_streq(cur, ";"))
			if (!CStream_nextBatch(scope->stream))
				return 0;
	}
	return 1;
}

static int add_function(CScope *scope, const char *name, CType to_add, CContext ctx)
{
	CType dup = CType_dup(to_add);
	CSymbol entry = CSymbol_init(CSYMBOL_FUNCTION, CType_primitiveData(dup));
	CSymbol had;

	dup.full->primitive.type = CPRIMITIVE_VOID;
	dup.full->primitive.data = NULL;
	CType_destroy(dup);
	if (name == NULL) {
		printf_error(ctx, "no name for function declaration");
		CSymbol_destroy(entry);
		return 0;
	}
	if (CType_refCount(to_add) > 0) {
		printf_error_part(ctx, "function declaration with reference: ");
		CType_print(to_add);
		printf("\n\n");
		CSymbol_destroy(entry);
		return 0;
	}
	if (CScope_resolve(scope, name, &had)) {
		if (!CSymbol_eq(entry, had)) {
			printf_error_symbol_redef(name, had, entry, ctx);
			CSymbol_destroy(entry);
			return 0;
		}
		CSymbol_destroy(entry);
		return 1;
	}
	if (!CScope_addSymbol(scope, name, entry, ctx)) {
		CSymbol_destroy(entry);
		return 0;
	}
	return 1;
}

static int parse_variable(CScope *scope)
{
	char *name;
	CType type;
	CStorageType storage;
	VecStr args;
	CToken cur;

	if (!CType_parseFull(scope, &name, &type, &storage, &args))
		return 0;
	if (!CStream_at(scope->stream, &cur)) {
		printf_error(CStream_lastCtx(scope->stream), "expected ; = { after variable declaration");
		free(name);
		CType_destroy(type);
		VecStr_destroy(args);
		return 0;
	}
	if (CToken_streq(cur, ";")) {
		if (name != NULL)
			printf("var %s: ", name);
		CType_print(type);
		printf("\n");

		if (CType_primitiveType(type) == CPRIMITIVE_FUNCTION)
			if (!add_function(scope, name, type, cur.ctx)) {
				free(name);
				CType_destroy(type);
				VecStr_destroy(args);
				return 0;
			}
		free(name);
		CType_destroy(type);
		VecStr_destroy(args);

		if (!CStream_nextBatch(scope->stream))
			return 0;
	} else if (CToken_streq(cur, "=")) {
		printf_error(cur.ctx, "unsupported assignment for now");
		free(name);
		CType_destroy(type);
		VecStr_destroy(args);
		return 0;
	} else if (CToken_streq(cur, "{")) {
		printf("fun %s: ", name);
		CType_print(type);
		printf("\n");

		free(name);
		CType_destroy(type);
		VecStr_destroy(args);

		skip_block(scope);
	} else {
		printf_error(cur.ctx, "expected ; = { after variable declaration");
		free(name);
		CType_destroy(type);
		VecStr_destroy(args);
		return 0;
	}
	return 1;
}

static int parse_typedef(CScope *scope)
{
	char *name;
	CType type;
	CToken cur;

	if (!CStream_poll(scope->stream, &cur))
		return 0;
	if (!CType_parseName(scope, &name, &type))
		return 0;
	if (!CStream_expectSemicolon(scope->stream)) {
		free(name);
		CType_destroy(type);
		return 0;
	}
	if (!add_type(scope, name, type, cur.ctx)) {
		free(name);
		CType_destroy(type);
		return 0;
	}

	printf("type %s: ", name);
	free(name);
	CType_print(type);
	printf("\n");
	return 1;
}

int try_poll_typedef(CScope *scope, int *is_err)
{
	CKeyword keyword;

	*is_err = 0;
	if (!CKeyword_at(scope, &keyword, NULL))
		return 0;
	if (keyword != CKEYWORD_TYPEDEF)
		return 0;
	if (!parse_typedef(scope)) {
		*is_err = 1;
		return 0;
	}
	return 1;
}

int try_poll_variables(CScope *scope, int *is_err)
{
	CToken cur;

	if (!CStream_atIsType(scope))
		return 0;
	if (!parse_variable(scope)) {
		*is_err = 1;
		return 0;
	}
	return 1;
}

int poll_statement(CScope *scope)
{
	CNode node;

	if (!CNode_poll(scope, ";", &node))
		return 0;
	terminal_flush();
	CNode_print(node);
	terminal_show();
	CNode_destroy(node);
	return 1;
}

int CParser_exec(const char *path)
{
	CScope *scope;
	int is_err = 0;

	if (!CScope_create(path, &scope))
		return 0;
	if (!CStream_nextBatch(scope->stream)) {
		CScope_destroy(scope);
		return 0;
	}
	while (!CStream_isEof(scope->stream)) {
		if (!try_poll_typedef(scope, &is_err)) {
			if (is_err) {
				CScope_destroy(scope);
				return 0;
			}
			if (!try_poll_variables(scope, &is_err)) {
				if (is_err) {
					CScope_destroy(scope);
					return 0;
				}
				if (!poll_statement(scope)) {
					CScope_destroy(scope);
					return 0;
				}
			}
		}
		if (CStream_isEof(scope->stream))
			break;
	}
	if (!CStream_ensureMacroStackEmpty(scope->stream)) {
		CScope_destroy(scope);
		return 0;
	}
	terminal_show();
	//StrSonic_print(scope->block[0].symbols);
	memcheck_stats();
	CScope_destroy(scope);
	return 1;
}

void CParser_destroy(CParser parser)
{
	CBuf_destroy(parser.buf);
	return;
}

int CKeyword_from_CToken(CToken src, CKeyword *pres)
{
	CSymbol sym;

	if (src.type != CTOKEN_BASIC)
		return 0;
	if (!StrSonic_resolveCSymbol(&keywords, src.str, &sym))
		return 0;
	if (sym.type != CSYMBOL_KEYWORD)
		return 0;
	*pres = (CKeyword)sym.data;
	return 1;
}

int CKeyword_at(CScope *scope, CKeyword *pres, CContext *ctx)
{
	CToken cur;
	CSymbol sym;

	if (!CStream_at(scope->stream, &cur))
		return 0;
	if (ctx != NULL)
		*ctx = cur.ctx;
	return CKeyword_from_CToken(cur, pres);
}

int CKeyword_poll(CScope *scope, CKeyword *pres, CContext *ctx)
{
	int res = CKeyword_at(scope, pres, ctx);

	if (res)
		CStream_forward(scope->stream);
	return res;
}

int CKeyword_isType(CKeyword k)
{
	return (k >= CKEYWORD_INLINE) && (k <= CKEYWORD_UNION);
}
