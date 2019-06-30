
#include "headers.h"

char* strdup(const char *src)
{
	size_t size = strlen(src) + 1;
	char *res = (char*)malloc(size * sizeof(char));

	memcpy(res, src, size * sizeof(char));
	return res;
}
