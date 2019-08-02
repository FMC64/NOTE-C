
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
		VecStr_add(&res, strdup(cur.str));
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

CMacro CMacro_null(void)
{
	CMacro res;

	res.argumentCount = 0;
	res.argumentBase = NULL;
	res.argumentCur = NULL;
	res.tokenBase = NULL;
	res.tokenCur = NULL;
	return res;
}

static int preproc_define(CStream *stream, VecCToken tokens, CContext ctx)
{
	CToken name;
	StreamCToken s = StreamCToken_init(tokens), save;
	VecStr args = VecStr_init();
	VecCToken macro_tokens;

	if (!StreamCToken_poll(&s, &name)) {
		printf_error(ctx, "expected a name for macro definition");
		return 0;
	}
	if (!CToken_isIdentifier(name)) {
		printf_error(name.ctx, "expected a valid identifier for macro name");
		return 0;
	}
	save = s;
	if (!define_get_args(&s, &args))
		s = save;
	macro_tokens = StreamCToken_offset(&s).vec;
	if (macro_tokens.count == 0) {
		s = save;
		macro_tokens = StreamCToken_offset(&s).vec;
	}
	StrSonic_add(&stream->macros, name.str, 0, CMacro_create(args, macro_tokens));
	VecStr_destroy(args);
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

static int resolve_macro(CStream *stream, const char *name, CMacro *pres)
{
	void *data;

	if (!StrSonic_resolve(&stream->macros, name, NULL, &data))
		return 0;
	*pres = CMacro_dump(data);
	return 1;
}

static int add_token(VecCToken *vec, CToken to_add, int *is_end)
{
	*is_end |= CToken_isEndBatch(to_add);
	VecCToken_add(vec, CToken_dup(to_add));
	return 1;
}

static int poll_args(StreamCTokenPoly *to_poll, VecVecCToken *pres, CToken macro, CContext ctx)
{
	VecVecCToken res = VecVecCToken_init();
	VecCToken arg = VecCToken_init();
	CToken cur;
	size_t level = 0;

	if (!StreamCTokenPoly_poll(to_poll, &cur)) {
		printf_error(ctx, "expected arguments for macro '%s'", macro.str);
		goto poll_args_error;
	}
	if (!CToken_streq(cur, "(")) {
		printf_error(ctx, "expected '(' to begin arguments for '%s'", macro.str);
		goto poll_args_error;
	}
	CToken_destroy(cur);
	while (1) {
		if (!StreamCTokenPoly_poll(to_poll, &cur)) {
			printf_error(ctx, "expected ')' for ending macro arguments for '%s'", macro.str);
			goto poll_args_error;
		}
		if (level == 0) {
			if (CToken_streq(cur, "("))
				level++;
			if (CToken_streq(cur, ")")) {
				CToken_destroy(cur);
				VecVecCToken_add(&res, arg);
				arg = VecCToken_init();
				break;
			} else if (CToken_streq(cur, ",")) {
				CToken_destroy(cur);
				VecVecCToken_add(&res, arg);
				arg = VecCToken_init();
			} else
				VecCToken_add(&arg, cur);
		} else {
			VecCToken_add(&arg, cur);
			if (CToken_streq(cur, "("))
				level++;
			if (CToken_streq(cur, ")")) {	// looks impossible ?
				if (level == 0) {
					printf_error(ctx, "misplaced parentheses for macro '%s' arguments (go sub level 0)", macro.str);
					goto poll_args_error;
				}
				level--;
			}
		}
	}
	*pres = res;
	return 1;

poll_args_error:
	VecVecCToken_destroy(res);
	VecCToken_destroy(arg);
	return 0;
}

static int resolve_arg(char *name, void *macroData, VecVecCToken macroArgs, VecCToken *pres)
{
	CMacro macro;
	char *arg;
	size_t i;

	if (macroData == NULL)
		return 0;
	macro = CMacro_dump(macroData);
	for (i = 0; CMacro_nextArgument(&macro, &arg); i++)
		if (streq(name, arg)) {
			*pres = macroArgs.vec[i];
			return 1;
		}
	return 0;
}

static int substitute_macro_ac(CStream *stream, CToken to_subs, VecCToken *dest, StreamCTokenPoly *to_poll, int *is_end, size_t depth, CContext first_token_ctx)
{
	CMacro macro;
	CToken cur;
	CToken new_token;
	VecVecCToken args = VecVecCToken_init();
	VecCToken arg;
	VecCToken macro_tokens = VecCToken_init();
	StreamCToken macro_stream_ctoken;
	StreamCTokenPoly macro_stream;
	size_t limitDepth = 64;
	size_t i;

	if (depth > limitDepth) {
		printf_error(first_token_ctx, "too deep macro ! (exceeds level %u)", limitDepth);
		return 0;
	}
	if (CToken_isString(to_subs))
		return add_token(dest, to_subs, is_end);
	if (!resolve_macro(stream, to_subs.str, &macro))
		return add_token(dest, to_subs, is_end);
	if (macro.argumentCount > 0)
		if (!poll_args(to_poll, &args, to_subs, first_token_ctx))
			return 0;
	if (args.count != macro.argumentCount) {
		if (args.count > macro.argumentCount)
			printf_error(first_token_ctx, "too much arguments for macro '%s' (%u expected, got %u)", to_subs.str, macro.argumentCount, args.count);
		if (args.count < macro.argumentCount)
			printf_error(first_token_ctx, "not enough arguments for macro '%s' (%u expected, got %u)", to_subs.str, macro.argumentCount, args.count);
		VecVecCToken_destroy(args);
		return 0;
	}

	while (CMacro_nextToken(&macro, &cur)) {
		new_token = CToken_dup(cur);
		new_token.ctx = first_token_ctx;
		if (resolve_arg(new_token.str, macro.argumentBase, args, &arg)) {
			for (i = 0; i < arg.count; i++)
				VecCToken_add(&macro_tokens, CToken_dup(arg.token[i]));
			CToken_destroy(new_token);
		} else
			VecCToken_add(&macro_tokens, new_token);
	}
	macro_stream_ctoken = StreamCToken_init(macro_tokens);
	macro_stream = StreamCTokenPoly_initFromStreamCToken(&macro_stream_ctoken);
	while (StreamCTokenPoly_poll(&macro_stream, &cur)) {
		if (!substitute_macro_ac(stream, cur, dest, &macro_stream, is_end, depth + 1, first_token_ctx)) {
			CToken_destroy(cur);
			VecCToken_destroy(macro_tokens);
			VecVecCToken_destroy(args);
			return 0;
		}
		CToken_destroy(cur);
	}
	VecCToken_destroy(macro_tokens);
	VecVecCToken_destroy(args);
	return 1;
}

int CStream_substituteMacro(CStream *stream, CToken to_subs, VecCToken *dest, StreamCTokenPoly to_poll, int *is_end)
{
	return substitute_macro_ac(stream, to_subs, dest, &to_poll, is_end, 0, to_subs.ctx);
}
