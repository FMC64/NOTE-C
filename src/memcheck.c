
#include "headers_ac.h"

static MemcheckBlock MemcheckBlock_init(void *ptr, Context ctx)
{
	MemcheckBlock res;

	res.ptr = ptr;
	res.ctx = ctx;
	return res;
}

static VecMemcheckBlock VecMemcheckBlock_init(void)
{
	VecMemcheckBlock res;

	res.count = 0;
	res.allocated = 0;
	res.block = NULL;
	return res;
}

static void VecMemcheckBlock_add(VecMemcheckBlock *vec, MemcheckBlock to_add)
{
	size_t cur;

	if (to_add.ptr == NULL)
		return;
	cur = vec->count++;
	if (vec->count > vec->allocated) {
		vec->allocated += 16;
		vec->block = (MemcheckBlock*)realloc(vec->block, vec->allocated * sizeof(MemcheckBlock));
	}
	vec->block[cur] = to_add;
}

static void VecMemcheckBlock_remove(VecMemcheckBlock *vec, size_t ndx)
{
	vec->count--;
	if (vec->count > 0)
		vec->block[ndx] = vec->block[vec->count];
}

static void VecMemcheckBlock_free(VecMemcheckBlock *vec, void *ptr, Context ctx)
{
	size_t i;

	if (ptr == NULL)
		return;
	for (i = 0; i < vec->count; i++) {
		if (vec->block[i].ptr == ptr) {
			VecMemcheckBlock_remove(vec, i);
			return;
		}
	}

	terminal_flush();

	printf_term("malloc error: block not found\n");
	printf_term("Can't free %p\n", ptr);
	Context_print_term(ctx);
	printf_term("%d blocks allocated\n\n", vec->count);
	printf_term("Press EXIT to continue");

	terminal_show();
	exit(1);
}

static void VecMemcheckBlock_destroy(VecMemcheckBlock blocks)
{
	free(blocks.block);
	return;
}

static VecMemcheckBlock blocks = {0, 0, NULL};

void* memcheck_malloc(size_t size, Context ctx)
{
	void *res = malloc(size);

	VecMemcheckBlock_add(&blocks, MemcheckBlock_init(res, ctx));
	return res;
}

void* memcheck_calloc(size_t member_count, size_t size, Context ctx)
{
	void *res = calloc(member_count, size);

	VecMemcheckBlock_add(&blocks, MemcheckBlock_init(res, ctx));
	return res;
}

void* memcheck_realloc(void *ptr, size_t new_size, Context ctx)
{
	void *res = realloc(ptr, new_size);

	VecMemcheckBlock_free(&blocks, ptr, ctx);
	VecMemcheckBlock_add(&blocks, MemcheckBlock_init(res, ctx));
	return res;
}

void memcheck_free(void *ptr, Context ctx)
{
	VecMemcheckBlock_free(&blocks, ptr, ctx);
	free(ptr);
	return;
}

void memcheck_recap(void)
{
	terminal_flush();
	if (blocks.count == 0)
		printf_term("malloc OK\n\n");
	else {
		printf_term("malloc error:\n");
		printf_term("net %d blocks allocated\n\n", blocks.count);
	}
	printf_term("Press EXIT to continue");
	terminal_show();
	VecMemcheckBlock_destroy(blocks);
	return;
}
