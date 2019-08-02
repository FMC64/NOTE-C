
#pragma once

CToken CToken_init(CTokenType type, const char *str, CContext ctx);
CToken CToken_dup(CToken src);
int CTokenType_isString(CTokenType type);
int CToken_isString(CToken token);
size_t CToken_stringSize(CToken token);
int CToken_isIdentifier(CToken token);
int CToken_isEndBatch(CToken token);
int CToken_streq(CToken token, const char *str);
void CToken_destroy(CToken token);

VecCToken VecCToken_init(void);
void VecCToken_add(VecCToken *vec, CToken to_add);
void VecCToken_print(VecCToken vec);
void VecCToken_display(VecCToken vec);
int VecCToken_at(VecCToken vec, size_t i, CToken *pres);
void VecCToken_flush(VecCToken *vec);
void VecCToken_moveArea(VecCToken *src, size_t src_start, size_t src_size, VecCToken *dst);
void VecCToken_merge(VecCToken *dst, VecCToken *to_append);
VecCToken VecCToken_offset(VecCToken vec, size_t off);
void VecCToken_destroy(VecCToken vec);

VecVecCToken VecVecCToken_init(void);
void VecVecCToken_add(VecVecCToken *vec, VecCToken to_add);
void VecVecCToken_print(VecVecCToken vec);
void VecVecCToken_destroy(VecVecCToken vec);

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

StreamCTokenPoly StreamCTokenPoly_initFromCStream(CStream *stream);
StreamCTokenPoly StreamCTokenPoly_initFromStreamCToken(StreamCToken *stream);
int StreamCTokenPoly_poll(StreamCTokenPoly *stream, CToken *pres);

int CStream_create(const char *filepath, CStream **pres);
void CStream_destroy(CStream *stream);
int CStream_currentStream(CStream *stream, CFile **pres);
int CStream_pollToken(CStream *stream, CToken *pres);
int CStream_nextBatch(CStream *stream);
int CStream_isEof(CStream *stream);
StreamCToken StreamCToken_init(VecCToken vec);
void StreamCToken_flush(StreamCToken *stream);
StreamCToken StreamCToken_offset(StreamCToken *stream);
void StreamCToken_destroy(StreamCToken stream);
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
void CStream_begin(CStream *stream);
void CStream_end(CStream *stream);
int CStream_forward(CStream *stream);
int CStream_back(CStream *stream);
int CStream_at(CStream *stream, CToken *pres);
int CStream_poll(CStream *stream, CToken *pres);
int CStream_pollRev(CStream *stream, CToken *pres);
CContext CStream_lastCtx(CStream *stream);
CContext CStream_atCtx(CStream *stream);
int CStream_pollStr(CStream *stream, const char *str, CContext *ctx);
int CStream_pollLpar(CStream *stream, CContext *ctx);
int CStream_pollRpar(CStream *stream, CContext *ctx);


CBuf CBuf_init(char *input_path);
int CBuf_readTokens(CBuf *buf);
void CBuf_destroy(CBuf buf);

char char_lower(char to_lower);
int char_is_letter(char to_test);
int char_is_digit(char to_test);
int char_is_identifier(char to_test);
int str_is_identifier(const char *str);


int CScope_keywords_init(void);
void CScope_keywords_quit(void);
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

int CStream_macro_init(void);
void CStream_macro_quit(void);
int CStream_parseMacro(CStream *stream, CToken macro);
int CStream_canAddToken(CStream *stream);
void StrSonic_CMacro_destroy(unsigned char type, void *data);

void* CMacro_create(VecStr args, VecCToken tokens);
int CMacro_nextArgument(CMacro *macro, char **pres);
int CMacro_nextToken(CMacro *macro, CToken *pres);
void CMacro_rewindArguments(CMacro *macro);
void CMacro_rewindTokens(CMacro *macro);
CMacro CMacro_dump(void *src);
CMacro CMacro_null(void);

int CStream_substituteMacro(CStream *stream, CToken to_subs, VecCToken *dest, StreamCTokenPoly to_poll, int *is_end);
