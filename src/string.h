
#pragma once

char* strdup(const char *src);
int streq(const char *a, const char *b);
int streq_part(const char *str, const char *part);
int streq_part_max(const char *str, const char *part, size_t *pmax);

Str Str_empty(void);
Str Str_init(size_t size, char *data);
Str Str_init_from_string(const char *src);
Str Str_create(size_t size, const char *data);
Str Str_create_from_string(const char *src);
void Str_append(Str *str, const Str to_add);
void Str_remove(Str *str, size_t start, size_t size);
int Str_char_escape(Str str, size_t *i, char *pres, CContext ctx);
int Str_escape(Str str, Str *pres, CContext ctx);
void Str_destroy(Str str);

char* string_create_from_Str(Str str);
char* Str_to_string(Str str);

VecStr VecStr_init(void);
void VecStr_add(VecStr *vec, const char *to_add);
void VecStr_print(VecStr vec);
void VecStr_destroy(VecStr vec);

StrSonic StrSonic_init(void (*elem_destroy_cb)(void*));
int StrSonic_add(StrSonic *sonic, const char *key, void *value);
void StrSonic_print(StrSonic sonic);
void** StrSonic_resolve_ref(StrSonic *sonic, const char *key);
void* StrSonic_resolve(StrSonic *sonic, const char *key);
void StrSonic_destroy_elem(StrSonic *sonic, const char *key);
void StrSonic_destroy(StrSonic *sonic);
