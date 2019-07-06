
#include "headers.h"

void Context_print(Context ctx, int *y)
{
	printf(file_shortpath(ctx.file));
	locate(1, (*y)++);
	printf("File: %s(%d)\n", file_shortpath(ctx.file), ctx.line);
}

void Context_print_term(Context ctx)
{
	printf_term("File: %s(%d)\n", file_shortpath(ctx.file), ctx.line);
}

CContext CContext_init(const char *file, int line, int colon)
{
	CContext res;

	res.file = file;
	res.line = line;
	res.colon = colon;
	return res;
}

void CContext_print_term(CContext ctx)
{
	printf_term("File: %s, line %d(%d)\n", ctx.file, ctx.line, ctx.colon);
}
