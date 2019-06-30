
#include "headers.h"

void Context_print(Context ctx, int *y)
{
	printf(file_shortpath(ctx.file));
	locate(1, (*y)++);
	printf("Line %d", ctx.line);
}
