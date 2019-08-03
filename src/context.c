
#include "headers.h"

void Context_print(Context ctx, int *y)
{
	printf(file_shortpath(ctx.file));
	locate(1, (*y)++);
	printf("File: %s(%d)\n", file_shortpath(ctx.file), ctx.line);
}

void Context_print_term(Context ctx)
{
	printf("File: %s(%d)\n", file_shortpath(ctx.file), ctx.line);
}

CContext CContext_init(const char *file, int line, int colon)
{
	CContext res;

	res.file = file;
	res.line = line;
	res.colon = colon;
	return res;
}

CContext CContext_null(void)
{
	return CContext_init("{undefined file}", 0, 0);
}

void CContext_print(CContext ctx)
{
	printf("%s line %d(%d)\n", ctx.file, ctx.line, ctx.colon);
}

CContext CContext_polled(CStream *stream)
{
	size_t i = stream->tokens.i;

	if (i > 0) {
		i--;
		return stream->tokens.vec.token[i].ctx;
	} else
		return CContext_null();
}

CContext CContext_dup(CContext src)
{
	CContext res;

	res = src;
	res.file = strdup(src.file);
	return res;
}

void CContext_destroy(CContext ctx)
{
	free(ctx.file);
}
