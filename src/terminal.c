
#include "headers.h"

static VecStr lines = {0, 0, NULL};
static Str buf = {0, NULL};

static void flush_buf(void)
{
	if (buf.size == 0)
		return;
	VecStr_add(&lines, string_create_from_Str(buf));
	Str_destroy(buf);
	buf = Str_empty();
}

void terminal_flush(void)
{
	flush_buf();
	VecStr_destroy(lines);
	lines = VecStr_init();
}

static int try_cut_buf(void)
{
	size_t i;

	for (i = 0; i < buf.size; i++)
		if (buf.data[i] == '\n') {
			VecStr_add(&lines, string_create_from_Str(Str_init(i, buf.data)));
			Str_remove(&buf, 0, i + 1);
			return 1;
		}
	return 0;
}

static buf_append(const char *to_add)
{
	Str_append(&buf, Str_init_from_string(to_add));
	while (try_cut_buf());
}

int printf_term(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	int res;

	va_start(args, fmt);
	res = vsprintf(buf, fmt, args);
	va_end(args);
	buf_append(buf);
	return res;
}

void terminal_show(void)
{
	ssize_t x = 0;
	ssize_t y = 0;
	size_t i;

	flush_buf();
	while (IsKeyDown(KEY_CTRL_EXIT));
	while (!IsKeyDown(KEY_CTRL_EXIT))
	{
		if (IsKeyDown(KEY_CTRL_UP))
			y--;
		if (IsKeyDown(KEY_CTRL_DOWN))
			y++;
		if (IsKeyDown(KEY_CTRL_LEFT))
			x--;
		if (IsKeyDown(KEY_CTRL_RIGHT))
			x++;

		y = CLAMP(y, 0, (ssize_t)(lines.count - 1 - 7));
		if (x < 0)
			x = 0;

		ML_clear_vram();

		for (i = 0; i < 8; i++) {
			if (y + i >= lines.count)
				continue;
			if (x >= strlen(lines.str[y + i]))
				continue;

			locate(1, 1 + i);
			Print((unsigned char*)&(lines.str[y + i][x]));
		}

		ML_display_vram();
		Sleep(100);
	}
	return;
}
