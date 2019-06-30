
#pragma once

char* strdup(const char *src);

Str Str_init(size_t size, char *data);
Str Str_create(size_t size, const char *data);
Str Str_fromString(const char *src);
void Str_destroy(Str str);
