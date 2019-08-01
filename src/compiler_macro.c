
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

static void* CMacro_create(VecStr args, VecCToken tokens)
{
	return NULL;
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
