
#include "headers.h"

VecStr VecStr_init(void)
{
	VecStr res;

	res.count = 0;
	res.allocated = 0;
	res.str = NULL;
	return res;
}

// Append by copy
void VecStr_add(VecStr *vec, const char *to_add)
{
	size_t cur = vec->count++;

	if (vec->count > vec->allocated) {
		vec->allocated += 16;
		vec->str = (char**)realloc(vec->str, vec->allocated * sizeof(char*));
	}
	vec->str[cur] = strdup(to_add);
	return;
}

void VecStr_destroy(VecStr vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++)
		free(vec.str[i]);
	free(vec.str);
	return;
}

CBuf CBuf_init(char *input_path)
{
	const FONTCHARACTER *path_real = string_to_fontchar(input_path);
	CBuf res;

	res.input_file_path = strdup(input_path);
	res.input_file = Bfile_OpenFile(path_real, _OPENMODE_READ);
	if (res.input_file < 0)
		fx_error(res.input_file, res.input_file_path);
	res.tokens = VecStr_init();
	free(path_real);
	return res;
}

void CBuf_getTokens(CBuf *buf)
{
	int size = Bfile_GetFileSize(buf->input_file);

	fx_assert(size, buf->input_file_path);
	
}

void CBuf_destroy(CBuf buf)
{
	free(buf.input_file_path);
	fx_assert(Bfile_CloseFile(buf.input_file), buf.input_file_path);
	VecStr_destroy(buf.tokens);
	return;
}

CParser CParser_init(char *source_path)
{
	CParser res;

	res.buf = CBuf_init(source_path);
	return res;
}

void CParser_exec(CParser *parser)
{
	CBuf_getTokens(&parser->buf);
}

void CParser_destroy(CParser parser)
{
	CBuf_destroy(parser.buf);
	return;
}

void CCompiler(char *path)
{
	CParser parser = CParser_init(path);

	CParser_exec(&parser);
	while (!IsKeyDown(KEY_CTRL_EXIT))
	{
		ML_clear_vram();
		locate(1, 1);
		printf("file: %d", parser.buf.input_file);
		ML_display_vram();
	}
	CParser_destroy(parser);
	return;
}
