
#pragma once

CToken CToken_init(CTokenType type, const char *str, CContext ctx);
void CToken_destroy(CToken token);

VecCToken VecCToken_init(void);
void VecCToken_add(VecCToken *vec, CToken to_add);
void VecCToken_print(VecCToken vec);
void VecCToken_print(VecCToken vec);
int VecCToken_at(VecCToken vec, size_t i, CToken *pres);
void VecCToken_flush(VecCToken *vec);
void VecCToken_moveArea(VecCToken *src, size_t src_start, size_t src_size, VecCToken *dst);
void VecCToken_merge(VecCToken *dst, VecCToken *to_append);
void VecCToken_destroy(VecCToken vec);

int CFile_create(const char *filepath, CFile *pres);
int CFile_pollFileBytes(CFile *stream, size_t buf_start, size_t size);
int VecCToken_from_CToken(const CToken src, VecCToken *pres);
int CFile_readToken(CFile *stream, CToken *pres, int *is_eof);
void CFile_destroy(CFile stream);

VecCFile VecCFile_init(void);
void VecCFile_add(VecCFile *vec, CFile to_add);
void VecCFile_flush(VecCFile *vec);
void VecCFile_move(VecCFile *src, size_t src_ndx, VecCFile *dst);
void VecCFile_moveArea(VecCFile *src, size_t src_start, size_t src_size, VecCFile *dst);
void VecCFile_destroy(VecCFile vec);

int CStream_create(const char *filepath, CStream **pres);
void CStream_destroy(CStream *stream);
int CStream_currentStream(CStream *stream, CFile **pres);
int CStream_nextBatch(CStream *stream);
int CStream_isEof(CStream *stream);
void CStream_begin(CStream *stream);
void CStream_end(CStream *stream);
int CStream_forward(CStream *stream);
int CStream_back(CStream *stream);
int CStream_at(CStream *stream, CToken *pres);
int CStream_poll(CStream *stream, CToken *pres);
int CStream_pollRev(CStream *stream, CToken *pres);
CContext CStream_lastCtx(CStream *stream);
CContext CStream_atCtx(CStream *stream);
int CStream_pollStr(CStream *tokens, const char *str, CContext *ctx);
int CStream_pollLpar(CStream *tokens, CContext *ctx);
int CStream_pollRpar(CStream *tokens, CContext *ctx);

CBuf CBuf_init(char *input_path);
int CBuf_readTokens(CBuf *buf);
void CBuf_destroy(CBuf buf);

char char_lower(char to_lower);
int char_is_letter(char to_test);
int char_is_digit(char to_test);
int char_is_identifier(char to_test);
int str_is_identifier(const char *str);


int CScope_create(const char *filepath, CScope **pres);
void CScope_addBlock(CScope *scope, CBlock to_add);
int CScope_removeBlock(CScope *scope, CContext ctx);
int CScope_addSymbol(CScope *scope, const char *key, CSymbol to_add, CContext ctx);
int CScope_resolve(CScope *scope, const char *key, CSymbol *pres);
void CScope_destroy(CScope *scope);

CBlock CBlock_default(void);
void CBlock_destroy(CBlock block);

CSymbol CSymbol_init(CSymbolType type, void *data);
void CSymbol_destroy(CSymbol symbol);
void StrSonic_CSymbol_destroy(unsigned char type, void *data);

CSymbol CKeyword_create(CKeyword keyword);
const char* CKeyword_str(CKeyword keyword);
void CKeyword_destroy(void *data);

CParser CParser_init(char *source_path);
int CParser_exec(const char *path);
void CParser_destroy(CParser parser);

int CKeyword_poll(CScope *scope, CKeyword *pres, CContext *ctx);


void CCompiler(const char *path);


CPrimitive CPrimitive_default(void);
void CPrimitive_destroy(CPrimitive primitive);


const char* CTypeFlag_str(CTypeFlag flag);
const char* CStorageType_str(CStorageType storage);

void CPrimitive_destroy(CPrimitive primitive);

CType CType_fromFull(CTypeFull *full);
CPrimitiveType CType_primitiveType(CType type);
void* CType_primitiveData(CType type);
size_t CType_referenceLevel(CType type);
void CType_destroy(CType type);

CTypeFull* CTypeFull_createPrimitive(CPrimitiveType type, size_t bits);

void CFunction_destroy(CFunction *func);

int CVariable_parse(CScope *scope, CVariable **pres, VecStr *pargs);
void CVariable_destroy(CVariable *variable);

int CTypeFull_parse(CScope *scope, char **pname, CTypeFull **pres, CStorageType *pstorage, VecStr *pargsName);
int CType_parseFull(CScope *scope, char **pname, CType *pres, CStorageType *pstorage, VecStr *pargsName);
int CType_parse(CScope *scope, CType *pres);
void CType_shrink(CScope *scope, CType *to_shrink);

CTypeFull* CTypeFull_alloc(CTypeFull base);
void CType_print_tree(CType type);
void CType_print(CType type);
void CTypeFull_destroy(CTypeFull *type);

char* CStruct_type(CStruct *str);
char* CStruct_name(CStruct *str);
