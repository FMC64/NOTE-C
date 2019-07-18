
#pragma once

CToken CToken_init(CTokenType type, const char *str, CContext ctx);
void CToken_destroy(CToken token);

VecCToken VecCToken_init(void);
void VecCToken_add(VecCToken *vec, CToken to_add);
void VecCToken_print(VecCToken vec);
void VecCToken_print(VecCToken vec);
int VecCToken_at(VecCToken vec, size_t i, CToken *pres);
void VecCToken_destroy(VecCToken vec);

StreamCToken StreamCToken_init(VecCToken vec);
void StreamCToken_begin(StreamCToken *stream);
void StreamCToken_end(StreamCToken *stream);
int StreamCToken_forward(StreamCToken *stream);
int StreamCToken_back(StreamCToken *stream);
int StreamCToken_at(StreamCToken *stream, CToken *pres);
int StreamCToken_poll(StreamCToken *stream, CToken *pres);
int StreamCToken_pollRev(StreamCToken *stream, CToken *pres);
CContext StreamCToken_lastCtx(StreamCToken *stream);
CContext StreamCToken_atCtx(StreamCToken *stream);
int StreamCToken_pollStr(StreamCToken *tokens, const char *str, CContext *ctx);
int StreamCToken_pollLpar(StreamCToken *tokens, CContext *ctx);
int StreamCToken_pollRpar(StreamCToken *tokens, CContext *ctx);

CBuf CBuf_init(char *input_path);
int CBuf_readTokens(CBuf *buf);
void CBuf_destroy(CBuf buf);

char char_lower(char to_lower);
int char_is_letter(char to_test);
int char_is_digit(char to_test);
int char_is_identifier(char to_test);
int str_is_identifier(const char *str);


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

int CKeyword_poll(CScope *scope, StreamCToken *tokens, CKeyword *pres, CContext *ctx);


void CCompiler(char *path);

CSymbol* CKeyword_create(CKeyword keyword);
void CKeyword_destroy(void *data);


CPrimitive CPrimitive_default(void);
void CPrimitive_destroy(CPrimitive primitive);


const char* CTypeFlag_str(CTypeFlag flag);
const char* CStorageType_str(CStorageType storage);

void CPrimitive_destroy(CPrimitive primitive);

CType CType_fromFull(CTypeFull *full);
void CType_destroy(CType type);

CTypeFull* CTypeFull_createPrimitive(CPrimitiveType type, size_t bits);

void CFunction_destroy(CFunction *func);

int CVariable_parse(CScope *scope, StreamCToken *tokens, CVariable **pres, VecStr *pargs);
void CVariable_destroy(CVariable *variable);

int CTypeFull_parse(CScope *scope, StreamCToken *tokens, char **pname, CTypeFull **pres, CStorageType *pstorage, VecStr *pargsName);
int CType_parse(CScope *scope, StreamCToken *tokens, char **pname, CType *pres, CStorageType *pstorage, VecStr *pargsName);
void CType_shrink(CScope *scope, CType *to_shrink);
CTypeFull* CTypeFull_alloc(CTypeFull base);
void CType_print(CType type);
void CTypeFull_destroy(CTypeFull *type);
