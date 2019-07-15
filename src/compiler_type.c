
#include "headers.h"

CType CType_default(void)
{
	CType res;

	res.flags = 0;
	res.referenceLevel = 0;
	res.primitiveType = CPRIMITIVE_INT;
	res.primitiveData = NULL;
	return res;
}

static const struct {CKeyword keyword; CTypeFlag flag;} keyword_flag[] = {
{CKEYWORD_SIGNED, CTYPE_SIGNED},
{CKEYWORD_UNSIGNED, CTYPE_UNSIGNED},
{CKEYWORD_CONST, CTYPE_CONST},
{CKEYWORD_VOLATILE, CTYPE_VOLATILE},
{CKEYWORD_NONE, CTYPE_NONE}};

static CKeyword CTypeFlag_keyword(CTypeFlag flag)
{
	size_t i;

	for (i = 0; keyword_flag[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_flag[i].flag == flag)
			return keyword_flag[i].keyword;
	return CKEYWORD_NONE;
}

const char* CTypeFlag_str(CTypeFlag flag)
{
	return CKeyword_str(CTypeFlag_keyword(flag));
}

static int get_flag(CKeyword keyword, CTypeFlag *flag)
{
	size_t i;

	for (i = 0; keyword_flag[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_flag[i].keyword == keyword) {
			*flag = keyword_flag[i].flag;
			return 1;
		}
	return 0;
}

static const struct {CKeyword keyword; CStorageType storage;} keyword_storage[] = {
{CKEYWORD_STATIC, CSTORAGE_STATIC},
{CKEYWORD_EXTERN, CSTORAGE_EXTERN},
{CKEYWORD_CONST, CTYPE_CONST},
{CKEYWORD_AUTO, CSTORAGE_AUTO},
{CKEYWORD_REGISTER, CSTORAGE_REGISTER},
{CKEYWORD_NONE, CSTORAGE_DEFAULT}};

static CKeyword CStorageType_keyword(CStorageType storage)
{
	size_t i;

	for (i = 0; keyword_storage[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_storage[i].storage == storage)
			return keyword_storage[i].keyword;
	return CKEYWORD_NONE;
}

const char* CStorageType_str(CStorageType storage)
{
	return CKeyword_str(CStorageType_keyword(storage));
}

static int get_storage(CKeyword keyword, CStorageType *storage)
{
	size_t i;

	for (i = 0; keyword_storage[i].keyword != CKEYWORD_NONE; i++)
		if (keyword_storage[i].keyword == keyword) {
			*storage = keyword_storage[i].storage;
			return 1;
		}
	return 0;
}

static int CType_poll_attributes(CScope *scope, VecCToken tokens, size_t *i, CType *pres, CStorageType *pstorage)
{
	CKeyword keyword;
	CTypeFlag flag;
	CStorageType storage;

	if (pstorage != NULL)
		*pstorage = CSTORAGE_DEFAULT;
	while (CKeyword_poll(scope, tokens, i, &keyword)) {
		if (get_flag(keyword, &flag))
			pres->flags |= flag;
		else if (get_storage(keyword, &storage)) {
			if (pstorage == NULL) {
				printf_error(CContext_polled(tokens, *i), "storage specifier '%s' in type definition",
				CStorageType_str(storage));
				return 0;
			}
			if (*pstorage != CSTORAGE_DEFAULT) {
				printf_error(CContext_polled(tokens, *i), "storage specifier redefinition: '%s' to '%s'",
				CStorageType_str(*pstorage), CStorageType_str(storage));
				return 0;
			}
			*pstorage = storage;
		} else
			break;
	}
	return 1;
}

static int CType_poll_primitive()
{

}

int CType_parse(CScope *scope, VecCToken tokens, size_t *i, CType *pres)
{
	CType res = CType_default();

	if (!CType_poll_attributes(scope, tokens, i, &res, NULL))
		return 0;
	terminal_flush();
	printf("flags: %d\nref: %u\nprim: %d\n", res.flags, res.referenceLevel, res.primitiveType);
	terminal_show();
	*pres = res;
	return 1;
}

CType* CType_alloc(CType base)
{
	CType *res = (CType*)malloc(sizeof(CType));

	*res = base;
	return res;
}
