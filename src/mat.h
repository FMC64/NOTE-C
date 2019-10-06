
#pragma once

uinterval uinterval_init(size_t start, size_t end);
uinterval uinterval_null(void);
int uinterval_isInside(uinterval inter, size_t value);
uinterval uinterval_merge(uinterval a, uinterval b);
void uinterval_print(uinterval inter);
