
#include "headers.h"

char* strdup(const char *src)
{
	size_t size = strlen(src) + 1;
	char *res = (char*)malloc(size * sizeof(char));

	memcpy(res, src, size * sizeof(char));
	return res;
}

// Data is referenced
Str Str_init(size_t size, char *data)
{
	Str res;

	res.size = size;
	res.data = data;
	return res;
}

// Data is copied
Str Str_create(size_t size, const char *data)
{
	Str res;

	res.size = size;
	res.data = (char*)malloc(size * sizeof(char));
	memcpy(res.data, data, size * sizeof(char));
	return res;
}

Str Str_fromString(const char *src)
{
	return Str_create(strlen(src), src);
}

void Str_destroy(Str str)
{
	free(str.data);
	return;
}
