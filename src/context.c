
#include "headers.h"

void Context_print(Context ctx, int *y)
{
	printf(file_shortpath(ctx.file));
	locate(1, (*y)++);
	printf("Line %d", ctx.line);
}

void Context_print_term(Context ctx)
{
	printf_term("File: %s\n", file_shortpath(ctx.file));
	printf_term("Line: %d\n", ctx.line);

}
