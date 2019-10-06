
#include "headers.h"

uinterval uinterval_init(size_t start, size_t end)
{
	uinterval res;

	res.start = start;
	res.end = end;
	return res;
}

uinterval uinterval_null(void)
{
	return uinterval_init(0, 0);
}

int uinterval_isInside(uinterval inter, size_t value)
{
	return (value >= inter.start) && (value < inter.end);
}

static size_t uinterval_size(uinterval inter)
{
	return inter.end - inter.start;
}

uinterval uinterval_merge(uinterval a, uinterval b)
{
	if (uinterval_size(a) == 0)
		return b;
	if (uinterval_size(b) == 0)
		return a;
	return uinterval_init(MIN(a.start, b.start), MAX(a.end, b.end));
}

void uinterval_print(uinterval inter)
{
	printf("uinterval: %u, %u\n", inter.start, inter.end);
}
