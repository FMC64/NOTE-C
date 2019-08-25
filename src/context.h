
#pragma once

static Context Context_init(const char *file, int line)
{
	Context res;

	res.file = file;
	res.line = line;
	return res;
}

#define Context_build Context_init(__FILE__, __LINE__)

void Context_print(Context ctx, int *y);
void Context_print_term(Context ctx);
CContext CContext_init(const char *file, int line, int colon);
CContext CContext_null(void);
CContext CContext_null_ptr(void);
void CContext_print(CContext ctx);
CContext CContext_polled(CStream *stream);
CContext CContext_dup(CContext src);
void CContext_destroy(CContext ctx);
