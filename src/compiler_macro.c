
#include "headers.h"

static int define_get_args(StreamCToken *stream, VecStr *pres)
{
	VecStr res = VecStr_init();
	CToken cur;
	int forceContinue = 0;

	if (!StreamCToken_pollStr(stream, "(", NULL))
		return 0;
	while ((!StreamCToken_pollStr(stream, ")", NULL)) || forceContinue) {
		if (!StreamCToken_poll(stream, &cur)) {
			VecStr_destroy(res);
			return 0;
		}
		if (!CToken_isIdentifier(cur)) {
			VecStr_destroy(res);
			return 0;
		}
		VecStr_add(&res, cur.str);
		forceContinue = StreamCToken_pollStr(stream, ",", NULL);
	}
	*pres = res;
	return 1;
}

static size_t get_token_size(CToken token)
{
	if (CToken_isString(token))
		return 1 + CToken_stringSize(token);
	else
		return 1 + strlen(token.str) + 1;
}

static size_t get_macro_size(VecStr args, VecCToken tokens)
{
	size_t res = 2;	// end bytes for array
	size_t i;

	for (i = 0; i < args.count; i++)
		res += 1 + strlen(args.str[i]) + 1;	// announce byte + end byte
	for (i = 0; i < tokens.count; i++)
		res += 1 + get_token_size(tokens.token[i]);	// announce byte
	return res;
}

static void write_token(void *buf, size_t *i, CToken to_write)
{
	size_t size;

	((char*)buf)[(*i)++] = to_write.type;
	if (CToken_isString(to_write)) {
		size = CToken_stringSize(to_write);
		memcpy(ptr_add(buf, *i), to_write.str, size);
		*i += size;
	} else {
		strcpy(ptr_add(buf, *i), to_write.str);
		*i += strlen(to_write.str) + 1;
	}
}

static void CMacro_print(void *data)
{
	CMacro macro = CMacro_dump(data);
	char *cur_str;
	CToken cur_token;

	terminal_flush();

	printf("args: ");
	while (CMacro_nextArgument(&macro, &cur_str))
		printf("'%s' ", cur_str);
	printf("\ntokens: ");
	while (CMacro_nextToken(&macro, &cur_token)) {
		if (CToken_isString(cur_token)) {
			printf("'");
			Str_print(Str_init_from_CToken(cur_token));
			printf("' ");
		} else
			printf("'%s' ", cur_token.str);
	}
	terminal_show();
}

void* CMacro_create(VecStr args, VecCToken tokens)
{
	size_t size = get_macro_size(args, tokens);
	void *res = malloc(size);
	size_t i = 0;
	size_t j;

	for (j = 0; j < args.count; j++) {
		((char*)res)[i++] = 1;
		strcpy(ptr_add(res, i), args.str[j]);
		i += strlen(args.str[j]) + 1;
	}
	((char*)res)[i++] = 0;	// null byte on array end
	for (j = 0; j < tokens.count; j++) {
		((char*)res)[i++] = 1;
		write_token(res, &i, tokens.token[j]);
	}
	((char*)res)[i++] = 0;	// null byte on array end
	return res;
}

int CMacro_nextArgument(CMacro *macro, char **pres)
{
	char *res;
	size_t len;

	if (*macro->argumentCur == 0)
		return 0;
	macro->argumentCur++;
	res = macro->argumentCur;
	macro->argumentCur += strlen(macro->argumentCur) + 1;
	if (pres != NULL)
		*pres = res;
	return 1;
}

int CMacro_nextToken(CMacro *macro, CToken *pres)
{
	CToken res;
	CTokenType type;
	size_t len;

	if (*macro->tokenCur == 0)
		return 0;
	macro->tokenCur++;
	type = *(char*)macro->tokenCur;
	res = CToken_init(type, macro->tokenCur + 1, CContext_null());
	macro->tokenCur += get_token_size(res);
	if (pres != NULL)
		*pres = res;
	return 1;
}

void CMacro_rewindArguments(CMacro *macro)
{
	macro->argumentCur = macro->argumentBase;
}

void CMacro_rewindTokens(CMacro *macro)
{
	macro->tokenCur = macro->tokenBase;
}

CMacro CMacro_dump(void *src)
{
	CMacro res;

	res.argumentCount = 0;
	res.argumentBase = src;
	CMacro_rewindArguments(&res);
	while (CMacro_nextArgument(&res, NULL))
		res.argumentCount++;
	res.tokenBase = res.argumentCur + 1;
	CMacro_rewindArguments(&res);
	CMacro_rewindTokens(&res);
	return res;
}

static int preproc_define(CStream *stream, VecCToken tokens, CContext ctx)
{
	CToken name;
	StreamCToken s = StreamCToken_init(tokens);
	VecStr args = VecStr_init();

	if (!StreamCToken_poll(&s, &name)) {
		printf_error(ctx, "expected a name for macro definition");
		return 0;
	}
	if (!CToken_isIdentifier(name)) {
		printf_error(name.ctx, "expected a valid identifier for macro name");
		return 0;
	}
	if (!define_get_args(&s, &args))
		s = StreamCToken_init(tokens);
	StrSonic_add(&stream->macros, name.str, 0, CMacro_create(args, StreamCToken_offset(&s).vec));
	free(args.str);
	return 1;
}

typedef int (*preproc_fun_t)(CStream*, VecCToken, CContext);
static const struct {const char *key; preproc_fun_t fun;} str_preproc[] = {
{"define", &preproc_define},
{NULL, NULL}};

static StrSonic preprocs;

static void preproc_destroy_cb(unsigned char type, void *data)
{
}

int CStream_macro_init(void)
{
	size_t i;

	preprocs = StrSonic_init(&preproc_destroy_cb);
	for (i = 0; str_preproc[i].key != NULL; i++)
		if (!StrSonic_add(&preprocs, str_preproc[i].key, 0, str_preproc[i].fun))
			return 0;
	return 1;
}

void CStream_macro_quit(void)
{
	StrSonic_destroy(&preprocs);
}

int CStream_parseMacro(CStream *stream, CToken macro)
{
	VecCToken tokens;
	CToken name;
	preproc_fun_t to_call;
	int res;

	if (!VecCToken_from_CToken(macro, &tokens))
		return 0;
	if (!VecCToken_at(tokens, 0, &name)) {
		printf_error(macro.ctx, "expected preprocessor directive");
		VecCToken_destroy(tokens);
		return 0;
	}
	if (name.type != CTOKEN_BASIC) {
		printf_error(macro.ctx, "expected valid identifier for preprocessor directive");
		VecCToken_destroy(tokens);
		return 0;
	}
	if (!StrSonic_resolve(&preprocs, name.str, NULL, (void**)&to_call)) {
		VecCToken_destroy(tokens);
		return 0;
	}
	res = to_call(stream, VecCToken_offset(tokens, 1), name.ctx);
	VecCToken_destroy(tokens);
	return res;
}

int CStream_canAddToken(CStream *stream)
{
	return 1;
}

void StrSonic_CMacro_destroy(unsigned char type, void *data)
{
	free(data);
}
