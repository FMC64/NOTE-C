
#pragma once

char* strdup(const char *src);

Str Str_empty(void);
Str Str_init(size_t size, char *data);
Str Str_init_from_string(const char *src);
Str Str_create(size_t size, const char *data);
Str Str_create_from_string(const char *src);
void Str_append(Str *str, const Str to_add);
void Str_remove(Str *str, size_t start, size_t size);
void Str_destroy(Str str);

char* string_create_from_Str(Str str);

VecStr VecStr_init(void);
void VecStr_add(VecStr *vec, const char *to_add);
void VecStr_destroy(VecStr vec);
