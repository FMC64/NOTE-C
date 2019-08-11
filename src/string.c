
#include "headers.h"

char* strdup(const char *src)
{
	size_t size;
	char *res;

	if (src == NULL)
		return NULL;
	size = strlen(src) + 1;
	res = (char*)malloc(size * sizeof(char));
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

int streq_part_max(const char *str, const char *part, size_t *pmax)
{
	size_t i;

	for (i = 0; 1; i++) {
		if (part[i] == 0)
			return 1;
		if (str[i] != part[i]) {
			*pmax = i;
			return 0;
		}
		if (str[i] == 0)
			return 1;
	}
}

char* strcat_dup(const char *a, const char *b)
{
	char *res = (char*)malloc(strlen(a) + strlen(b) + 1);

	strcpy(res, a);
	strcat(res, b);
	return res;
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

Str Str_init_from_CToken(CToken token)
{
	return Str_init(*(size_t*)token.str, token.str + sizeof(size_t));
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

void Str_prepend(Str *str, const Str to_pre)
{
	size_t new_size = str->size + to_pre.size;
	size_t i;
	size_t i_rev;

	str->data = (char*)realloc(str->data, new_size * sizeof(char));
	for (i = 0; i < str->size; i++) {
		i_rev = str->size - i - 1;
		str->data[i_rev + to_pre.size] = str->data[i_rev];
	}
	memcpy(str->data, to_pre.data, to_pre.size * sizeof(char));
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

Str Str_dup(Str str)
{
	char *data = (char*)malloc((str.size + 1) * sizeof(char));

	memcpy(data, str.data, str.size * sizeof(char));
	return Str_init(str.size, data);
}

int Str_char_escape(Str str, size_t *i, char *pres, CContext ctx)
{
	CContext ctx_cur = ctx;

	if (str.data[*i] == '\\') {
		(*i)++;
		ctx_cur.colon += *i;
		if ((*i) >= str.size) {
			printf_error(ctx_cur, "Escaping sequence with no control character.\n");
			return 0;
		}
		ctx_cur.colon++;
		switch (str.data[(*i)++]) {
		case 'a':
			*pres = 0x07;
		case 'b':
			*pres = 0x08;
			break;
		case 'e':
			*pres = 0x1B;
			break;
		case 'f':
			*pres = 0x0C;
			break;
		case 'n':
			*pres = 0x0A;
			break;
		case 'r':
			*pres = 0x0D;
			break;
		case 't':
			*pres = 0x09;
			break;
		case 'v':
			*pres = 0x0B;
			break;
		case '\\':
			*pres = 0x5C;
			break;
		case '\'':
			*pres = 0x27;
			break;
		case '\"':
			*pres = 0x22;
			break;
		case '\?':
			*pres = 0x3F;
			break;
		case '0':
			*pres = 0x00;
			break;
		default:
			printf_error(ctx_cur, "Unrecognized escaping character: '%c'.\n", str.data[(*i) - 1]);
			return 0;
		}
	} else
		*pres = str.data[(*i)++];
	return 1;
}

int Str_escape(Str str, Str *pres, CContext ctx)
{
	Str res = Str_dup(str);
	size_t i;
	size_t pos = 0;

	for (i = 0; i < str.size;)
		if (!Str_char_escape(str, &i, &res.data[pos++], ctx)) {
			Str_destroy(res);
			return 0;
		}
	res.size = pos;
	*pres = res;
	return 1;
}

void Str_print(Str str)
{
	size_t i;

	for (i = 0; i < str.size; i++) {
		if (str.data[i] == 0)
			printf("\\0");
		else
			printf("%c", str.data[i]);
	}
}

int Str_eq(Str a, Str b)
{
	if (a.size != b.size)
		return 0;
	return memcmp(a.data, b.data, a.size) == 0;
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

char* Str_to_string(Str str)
{
	char *res = string_create_from_Str(str);

	Str_destroy(str);
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

	printf("VecStr: %u entries\n", vec.count);
	for (i = 0; i < vec.count; i++)
		printf("'%s' ", vec.str[i]);

	terminal_show();
}

int VecStr_at(VecStr vec, size_t i, char **pres)
{
	if (i < vec.count) {
		if (pres != NULL)
			*pres = vec.str[i];
		return 1;
	}
	return 0;
}

int VecStr_poll(VecStr vec, size_t *i, char **pres)
{
	int res;

	res = VecStr_at(vec, *i, pres);
	if (res)
		(*i)++;
	return res;
}

void VecStr_destroy(VecStr vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		free(vec.str[i]);
	free(vec.str);
	return;
}
