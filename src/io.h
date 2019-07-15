
#pragma once

#define fx_error(code, context) fx_error_real(code, context, Context_build)
#define fx_assert(code, context) fx_assert_real(code, context, Context_build)

int printf_locate(const char *fmt, ...);

FONTCHARACTER* string_to_fontchar(const char *src);
const char* file_shortpath(const char *src);

void fx_error_real(int code, const char *context, Context ctx);
void fx_assert_real(int code, const char *context, Context ctx);
