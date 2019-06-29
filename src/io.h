
#pragma once

#define fx_error(code, context) fx_error_real(code, context, __FILE__, __LINE__)
#define fx_assert(code, context) fx_assert_real(code, context, __FILE__, __LINE__)

FONTCHARACTER* string_to_fontchar(const char *src);
void fx_error_real(int code, char *context, char *file, int line);
