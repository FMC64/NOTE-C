
#pragma once

void terminal_flush(void);
int printf(const char *fmt, ...);
int printf_error_part(CContext ctx, const char *fmt, ...);
int printf_error(CContext ctx, const char *fmt, ...);
int printf_error_symbol_redef(const char *name, CSymbol had, CSymbol got, CContext ctx);
int printf_error_uinterval(CScope *scope, uinterval ctx, const char *fmt, ...);
int printf_error_uinterval_part(CScope *scope, uinterval ctx, const char *fmt, ...);
void terminal_show(void);
