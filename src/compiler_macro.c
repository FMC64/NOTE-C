
#include "headers.h"

static int preproc_define(CStream *stream, VecCToken tokens)
{
	VecCToken_print(tokens);
	return 1;
}

static const struct {const char *key; int (*fun)(CStream*, VecCToken);} str_preproc[] = {
{"define", preproc_define},
{NULL, NULL}};

static StrSonic preprocs;

static void preproc_destroy_cb(unsigned char type, void *data)
{
}

int CMacro_init(void)
{
	size_t i;

	preprocs = StrSonic_init(&preproc_destroy_cb);
	for (i = 0; str_preproc[i].key != NULL; i++)
		if (!StrSonic_add(&preprocs, str_preproc[i].key, 0, str_preproc[i].fun))
			return 0;
	return 1;
}

void CMacro_quit(void)
{
	StrSonic_destroy(&preprocs);
}

int CMacro_parse(CStream *stream, CToken macro)
{
	VecCToken tokens;
	CToken name;
	int (*to_call)(CStream*, VecCToken);
	int res;

	if (!VecCToken_from_CToken(macro, &tokens))
		return 0;
	if (!VecCToken_at(tokens, 0, &name)) {
		printf_error(macro.ctx, "expected preprocessor directive");
		VecCToken_destroy(tokens);
		return 0;
	}
	if (name.type != CTOKEN_IDENTIFIER) {
		printf_error(macro.ctx, "expected name for preprocessor directive");
		VecCToken_destroy(tokens);
		return 0;
	}
	if (!StrSonic_resolve(&preprocs, name.str, NULL, &to_call)) {
		VecCToken_destroy(tokens);
		return 0;
	}
	res = to_call(stream, VecCToken_offset(tokens, 1));
	VecCToken_destroy(tokens);
	return res;
}

int CMacro_canAddToken(CStream *stream)
{

}
