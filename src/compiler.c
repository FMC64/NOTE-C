
#include "headers.h"

void CCompiler(char *path)
{
	const FONTCHARACTER *path_real = string_to_fontchar(path);
	int file = Bfile_OpenFile(path_real, _OPENMODE_READ);

	free(path_real);
	if (file < 0)
		fx_error(file, path);
	while (!IsKeyDown(KEY_CTRL_EXIT))
	{
		ML_clear_vram();
		locate(1, 1);
		printf("file: %p", file);
		ML_display_vram();
	}
}
