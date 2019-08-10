
#pragma once

void terminal_flush(void);
int printf(const char *fmt, ...);
int printf_error_part(CContext ctx, const char *fmt, ...);
int printf_error(CContext ctx, const char *fmt, ...);
int printf_error_symbol_redef(const char *name, CSymbol had, CSymbol got, CContext ctx);
void terminal_show(void);
