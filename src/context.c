
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
