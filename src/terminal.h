
#pragma once

void terminal_flush(void);
int printf(const char *fmt, ...);
int printf_error_part(CContext ctx, const char *fmt, ...);
int printf_error(CContext ctx, const char *fmt, ...);
void terminal_show(void);
