
#include "headers.h"

char* strdup(const char *src)
{
	size_t size = strlen(src) + 1;
	char *res = (char*)malloc(size * sizeof(char));

	memcpy(res, src, size * sizeof(char));
	return res;
}

int streq(const char *a, const char *b)
{
	size_t i;

	for (i = 0; 1; i++) {
		if (a[i] != b[i])
			return 0;
		if (a[i] == 0)
			return 1;
	}
}

int streq_part(const char *str, const char *part)
{
	size_t i;

	for (i = 0; 1; i++) {
		if (part[i] == 0)
			return 1;
		if (str[i] != part[i])
			return 0;
		if (str[i] == 0)
			return 1;
	}
}

Str Str_empty(void)
{
	Str res;

	res.size = 0;
	res.data = NULL;
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

Str Str_init_from_string(const char *src)
{
	return Str_init(strlen(src), src);
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

Str Str_create_from_string(const char *src)
{
	return Str_create(strlen(src), src);
}

void Str_append(Str *str, const Str to_add)
{
	size_t new_size = str->size + to_add.size;

	str->data = (char*)realloc(str->data, new_size * sizeof(char));
	memcpy(&str->data[str->size], to_add.data, to_add.size * sizeof(char));
	str->size = new_size;
	return;
}

void Str_remove(Str *str, size_t start, size_t size)
{
	size_t i;

	for (i = 0; start + i + size < str->size; i++)
		str->data[start + i] = str->data[start + i + size];
	str->size -= size;
}

void Str_destroy(Str str)
{
	free(str.data);
	return;
}

char* string_create_from_Str(Str str)
{
	char *res = (char*)malloc((str.size + 1) * sizeof(char));

	memcpy(res, str.data, str.size * sizeof(char));
	res[str.size] = 0;
	return res;
}

VecStr VecStr_init(void)
{
	VecStr res;

	res.count = 0;
	res.allocated = 0;
	res.str = NULL;
	return res;
}

// Append by reference
void VecStr_add(VecStr *vec, const char *to_add)
{
	size_t cur = vec->count++;

	if (vec->count > vec->allocated) {
		vec->allocated += 16;
		vec->str = (char**)realloc(vec->str, vec->allocated * sizeof(char*));
	}
	vec->str[cur] = to_add;
	return;
}

void VecStr_print(VecStr vec)
{
	size_t i;

	terminal_flush();

	printf_term("Tokens: (%u)\n", vec.count);
	for (i = 0; i < vec.count; i++)
		printf_term("'%s' ", vec.str[i]);

	terminal_show();
}

void VecStr_destroy(VecStr vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		free(vec.str[i]);
	free(vec.str);
	return;
}
