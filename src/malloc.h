
#pragma once

void malloc_unified_init(void);
void* malloc_unified(size_t size);
void* calloc_unified(size_t member_count, size_t size);
void* realloc_unified(void *ptr, size_t size);
void* free_unified(void *ptr);

#ifndef MEMCHECK

#define malloc(size) malloc_unified(size)
#define calloc(member_count, size) calloc_unified(member_count, size)
#define realloc(ptr, new_size) realloc_unified(ptr, new_size)
#define free(ptr) free_unified(ptr)

#endif
