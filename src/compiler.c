
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

	res.tokens = VecStr_init();
	res.input_file = Bfile_OpenFile(path_real, _OPENMODE_READ);
	free(path_real);
	if (res.input_file < 0)
		fx_error(res.input_file, input_path);
	return res;
}

void CBuf_destroy(CBuf buf)
{
	VecStr_destroy(buf.tokens);
	fx_assert(Bfile_CloseFile(buf.input_file), NULL);
	return;
}

CParser CParser_init(char *source_path)
{
	CParser res;

	res.buf = CBuf_init(source_path);
	return res;
}

void CParser_destroy(CParser parser)
{
	CBuf_destroy(parser.buf);
	return;
}

void CCompiler(char *path)
{
	CParser parser = CParser_init(path);

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
