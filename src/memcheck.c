
#include "headers_ac.h"

static MemcheckBlock MemcheckBlock_init(void *ptr, size_t size, Context ctx)
{
	MemcheckBlock res;

	res.ptr = ptr;
	res.size = size;
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
	exit(0);
}

static void VecMemcheckBlock_destroy(VecMemcheckBlock blocks)
{
	free(blocks.block);
	return;
}

static VecMemcheckBlock blocks = {0, 0, NULL};
static is_disabled = 0;

static void alloc_check(size_t size, void *ptr, Context ctx)
{
	if (!((ptr == NULL) && (size > 0)))
		return;

	terminal_flush();

	printf_term("malloc error: block can't be allocated\n");
	printf_term("Returned null for %u bytes\n", size);
	Context_print_term(ctx);
	printf_term("%d blocks allocated\n\n", blocks.count);
	printf_term("Press EXIT to continue");

	terminal_show();
	exit(0);
}

void* memcheck_malloc(size_t size, Context ctx)
{
	void *res = malloc(size);

	alloc_check(size, res, ctx);
	if (!is_disabled)
		VecMemcheckBlock_add(&blocks, MemcheckBlock_init(res, size, ctx));
	return res;
}

void* memcheck_calloc(size_t member_count, size_t size, Context ctx)
{
	void *res = calloc(member_count, size);
	size_t size_ac = member_count * size;

	alloc_check(size_ac, res, ctx);
	if (!is_disabled)
		VecMemcheckBlock_add(&blocks, MemcheckBlock_init(res, size_ac, ctx));
	return res;
}

void* memcheck_realloc(void *ptr, size_t new_size, Context ctx)
{
	void *res = realloc(ptr, new_size);

	alloc_check(new_size, res, ctx);
	if (!is_disabled) {
		VecMemcheckBlock_free(&blocks, ptr, ctx);
		VecMemcheckBlock_add(&blocks, MemcheckBlock_init(res, new_size, ctx));
	}
	return res;
}

void memcheck_free(void *ptr, Context ctx)
{
	if (!is_disabled)
		VecMemcheckBlock_free(&blocks, ptr, ctx);
	free(ptr);
	return;
}

static void MemcheckBlock_print(MemcheckBlock block)
{
	printf_term("%p: %u bytes\n", block.ptr, block.size);
	Context_print_term(block.ctx);
}

static void VecMemcheckBlock_print(VecMemcheckBlock vec)
{
	size_t i;

	for (i = 0; i < vec.count; i++) {
		MemcheckBlock_print(vec.block[i]);
		printf_term("\n");
	}
}

void memcheck_recap(void)
{
	terminal_flush();
	is_disabled = 1;
	if (blocks.count == 0)
		printf_term("malloc OK\n\n");
	else {
		printf_term("malloc error:\n");
		printf_term("net %d blocks allocated\n\n", blocks.count);
		VecMemcheckBlock_print(blocks);
	}
	printf_term("Press EXIT to continue");
	terminal_show();
	VecMemcheckBlock_destroy(blocks);
	return;
}

void memcheck_stats(void)
{
	size_t i;
	size_t total = 0;
	size_t overhead = blocks.count * 4;
	size_t memcheck_overhead = blocks.count * sizeof(MemcheckBlock);

	for (i = 0; i < blocks.count; i++)
		total += blocks.block[i].size;

	terminal_flush();

	printf_term("%u blocks\n%u bytes allocated\noverhead: %u bytes\nmemcheck overhead: %u bytes\n\nTOTAL: %u bytes\n",
	blocks.count, total, overhead, memcheck_overhead, total + overhead + memcheck_overhead);

	terminal_show();
}
