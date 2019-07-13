
#pragma once

CBuf CBuf_init(char *input_path);
int CBuf_readTokens(CBuf *buf);
void CBuf_destroy(CBuf buf);

CParser CParser_init(char *source_path);
int CParser_exec(CParser *parser);
void CParser_destroy(CParser parser);

void CCompiler(char *path);

CSymbol* CKeyword_create(CKeyword keyword);
void CKeyword_destroy(void *data);
