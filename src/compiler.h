
#pragma once

CToken CToken_init(CTokenType type, const char *str, CContext ctx);
void CToken_destroy(CToken token);

VecCToken VecCToken_init(void);
void VecCToken_add(VecCToken *vec, CToken to_add);
void VecCToken_print(VecCToken vec);
void VecCToken_print(VecCToken vec);
int VecCToken_at(VecCToken vec, size_t i, CToken *pres);
int VecCToken_poll(VecCToken vec, size_t *i, CToken *pres);
void VecCToken_destroy(VecCToken vec);

CBuf CBuf_init(char *input_path);
int CBuf_readTokens(CBuf *buf);
void CBuf_destroy(CBuf buf);


CScope* CScope_create(void);
void CScope_addBlock(CScope *scope, CBlock to_add);
int CScope_removeBlock(CScope *scope, CContext ctx);
int CScope_addSymbol(CScope *scope, const char *key, CSymbol *to_add, CContext ctx);
int CScope_resolve(CScope *scope, const char *key, void **pres);
void CScope_destroy(CScope *scope);

CBlock CBlock_default(void);
void CBlock_destroy(CBlock block);

CSymbol* CSymbol_create(CSymbolType type, void *data);
void CSymbol_destroy(CSymbol *symbol);

CSymbol* CKeyword_create(CKeyword keyword);
const char* CKeyword_str(CKeyword keyword);
void CKeyword_destroy(void *data);

CParser CParser_init(char *source_path);
int CParser_exec(CParser *parser);
void CParser_destroy(CParser parser);

int CKeyword_poll(CScope *scope, VecCToken tokens, size_t *i, CKeyword *pres);


void CCompiler(char *path);

CSymbol* CKeyword_create(CKeyword keyword);
void CKeyword_destroy(void *data);

const char* CTypeFlag_str(CTypeFlag flag);
const char* CStorageType_str(CStorageType storage);

int CType_parse(CScope *scope, VecCToken tokens, size_t *i, CType *pres);
CType* CType_alloc(CType base);
