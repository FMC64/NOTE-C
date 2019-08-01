
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
	unsigned char type;
	void *data;
	size_t i;
	size_t i_rev;

	for (i = 0; i < scope->blockCount; i++) {
		i_rev = scope->blockCount - 1 - i;
		data = StrSonic_resolve(&scope->block[i_rev].symbols, key, &type);
		if (data != NULL) {
			*pres = CSymbol_init(type, data);
			return 1;
		}
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

void CSymbol_destroy(CSymbol symbol)
{
	switch (symbol.type) {
	case CSYMBOL_KEYWORD:
		CKeyword_destroy(symbol.data);
		break;
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

static int populate_keywords(CScope *scope)
{

	size_t i;

	for (i = 0; str_keyword[i].key != NULL; i++)
		if (!CScope_addSymbol(scope, str_keyword[i].key, CKeyword_create(str_keyword[i].keyword), CContext_null()))
			return 0;
	return 1;
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
	int is_Str = 0;

	if (CStream_at(scope->stream, &cur)) {
		str = cur.str;
		if (CToken_isString(cur))
			is_Str = 1;
	}
	if (is_Str) {
		printf_error_part(CStream_atCtx(scope->stream), "unexpected token: '", str);
		Str_print(Str_init_from_CToken(cur));
		printf("'\n\n");
	} else
		printf_error(CStream_atCtx(scope->stream), "unexpected token: '%s'", str);
}

int CParser_exec(const char *path)
{
	CScope *scope;
	int res = 1;
	CKeyword keyword;
	char *name;
	CType type;

	if (!CScope_create(path, &scope)) {
		CScope_destroy(scope);
		return 0;
	}
	if (!populate_keywords(scope)) {
		CScope_destroy(scope);
		return 0;
	}
	if (!CStream_nextBatch(scope->stream)) {
		CScope_destroy(scope);
		return 0;
	}
	while (!CStream_isEof(scope->stream)) {
		if (CKeyword_poll(scope, &keyword, NULL)) {
			switch (keyword) {
			case CKEYWORD_TYPEDEF:
				if (!CType_parseFull(scope, &name, &type, NULL, NULL)) {
					CScope_destroy(scope);
					return 0;
				}
				CType_print(type);
				printf("\n");
				free(name);
				CType_destroy(type);
				break;
			default:
				print_error_unexp_token_at(scope);
				CScope_destroy(scope);
				return 0;
			}
		} else {
			print_error_unexp_token_at(scope);
			CScope_destroy(scope);
			return 0;
		}
		if (!CStream_nextBatch(scope->stream)) {
			CScope_destroy(scope);
			return 0;
		}
	}
	CScope_destroy(scope);
	return 1;
}

void CParser_destroy(CParser parser)
{
	CBuf_destroy(parser.buf);
	return;
}

int CKeyword_poll(CScope *scope, CKeyword *pres, CContext *ctx)
{
	CToken cur;
	CSymbol sym;

	if (!CStream_at(scope->stream, &cur))
		return 0;
	if (!CScope_resolve(scope, cur.str, &sym))
		return 0;
	if (sym.type != CSYMBOL_KEYWORD)
		return 0;
	*pres = (CKeyword)sym.data;
	if (ctx != NULL)
		*ctx = cur.ctx;
	CStream_forward(scope->stream);
	return 1;
}
