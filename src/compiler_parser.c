
#include "headers.h"

CSymbol* CSymbol_create(CSymbolType type, void *data)
{
	CSymbol *res = (CSymbol*)malloc(sizeof(CSymbol));

	res->type = type;
	res->data = data;
	return res;
}

void CSymbol_destroy(CSymbol *symbol)
{
	switch (symbol->type) {
	case CSYMBOL_KEYWORD:
		CKeyword_destroy(symbol->data);
		break;
	}
	free(symbol);
}

CSymbol* CKeyword_create(CKeyword keyword)
{
	return CSymbol_create(CSYMBOL_KEYWORD, (void*)keyword);
}

void CKeyword_destroy(void *data)
{
}

/*CType* CType_create()
{

}*/

CParser CParser_init(char *source_path)
{
	CParser res;

	res.buf = CBuf_init(source_path);
	return res;
}

static void populate_keywords(StrSonic *sonic)
{
	struct {const char *key; CKeyword keyword;} entry[] = {
	{"auto", CKEYWORD_AUTO},
	{"break", CKEYWORD_BREAK},
	{"case", CKEYWORD_CASE},
	{"char", CKEYWORD_CHAR},
	{"const", CKEYWORD_CONST},
	{"continue", CKEYWORD_CONTINUE},
	{"default", CKEYWORD_DEFAULT},
	{"do", CKEYWORD_DO},
	{"double", CKEYWORD_DOUBLE},
	{"else", CKEYWORD_ELSE},
	{"enum", CKEYWORD_ENUM},
	{"extern", CKEYWORD_EXTERN},
	{"float", CKEYWORD_FLOAT},
	{"for", CKEYWORD_FOR},
	{"goto", CKEYWORD_GOTO},
	{"if", CKEYWORD_IF},
	{"inline", CKEYWORD_INLINE},
	{"int", CKEYWORD_INT},
	{"long", CKEYWORD_LONG},
	{"register", CKEYWORD_REGISTER},
	{"return", CKEYWORD_RETURN},
	{"short", CKEYWORD_SHORT},
	{"signed", CKEYWORD_SIGNED},
	{"sizeof", CKEYWORD_SIZEOF},
	{"static", CKEYWORD_STATIC},
	{"struct", CKEYWORD_STRUCT},
	{"switch", CKEYWORD_SWITCH},
	{"typedef", CKEYWORD_TYPEDEF},
	{"union", CKEYWORD_UNION},
	{"unsigned", CKEYWORD_UNSIGNED},
	{"void", CKEYWORD_VOID},
	{"volatile", CKEYWORD_VOLATILE},
	{"while", CKEYWORD_WHILE},
	{NULL, CKEYWORD_NONE}};
	size_t i;

	for (i = 0; entry[i].key != NULL; i++)
		StrSonic_add(sonic, entry[i].key, CKeyword_create(entry[i].keyword));
}

int CParser_exec(CParser *parser)
{
	StrSonic sonic = StrSonic_init((void (*)(void*))&CSymbol_destroy);

	if (!CBuf_readTokens(&parser->buf))
		return 0;
	populate_keywords(&sonic);
	StrSonic_print(sonic);
	StrSonic_destroy(&sonic);
	return 1;
}

void CParser_destroy(CParser parser)
{
	CBuf_destroy(parser.buf);
	return;
}
