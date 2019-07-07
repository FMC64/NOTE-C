
#pragma once

void* memcheck_malloc(size_t size, Context ctx);
void* memcheck_calloc(size_t member_count, size_t size, Context ctx);
void* memcheck_realloc(void *ptr, size_t new_size, Context ctx);
void memcheck_free(void *ptr, Context ctx);

void memcheck_recap(void);
void memcheck_stats(void);

void memcheck_rnd_alloc(void);
void memcheck_test_limit(void);

#ifdef MEMCHECK

#define malloc(size) memcheck_malloc(size, Context_build)
#define calloc(member_count, size) memcheck_calloc(member_count, size, Context_build)
#define realloc(ptr, new_size) memcheck_realloc(ptr, new_size, Context_build)
#define free(ptr) memcheck_free(ptr, Context_build)

#endif
