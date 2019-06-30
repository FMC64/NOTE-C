
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI (float)3.14159265358979323846
#endif

#include <fxlib.h>

#include "MonochromeLib.h"

#include "struct.h"

#include "memcheck.h"
#include "string.h"
#include "io.h"
#include "compiler.h"

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
	int y;

	if (ptr == NULL)
		return;
	for (i = 0; i < vec->count; i++) {
		if (vec->block[i].ptr == ptr) {
			VecMemcheckBlock_remove(vec, i);
			return;
		}
	}
	while (IsKeyDown(KEY_CTRL_EXIT));
	while (!IsKeyDown(KEY_CTRL_EXIT))
	{
		ML_clear_vram();

		y = 1;

		locate(1, y++);
		printf("malloc error:");
		locate(1, y++);
		printf("block not found");
		locate(1, y++);
		printf("Can't free %p", ptr);
		locate(1, y++);
		Context_print(ctx, &y);

		locate(1, y++);
		printf("%d blocks", vec->count);
		locate(1, y++);
		printf("Press EXIT");
		locate(1, y++);
		printf("to continue");

		ML_display_vram();
		Sleep(100);
	}
	abort(0);
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
	int y;

	while (IsKeyDown(KEY_CTRL_EXIT));
	while (!IsKeyDown(KEY_CTRL_EXIT))
	{
		ML_clear_vram();

		y = 1;
		if (blocks.count == 0) {
			locate(1, y++);
			printf("malloc OK");
		} else {
			locate(1, y++);
			printf("malloc error:");
			locate(1, y++);
			printf("net %d blocks", blocks.count);
			locate(1, y++);
			printf("allocated");
		}

		y++;
		locate(1, y++);
		printf("Press EXIT");
		locate(1, y++);
		printf("to continue");

		ML_display_vram();
		Sleep(100);
	}
	VecMemcheckBlock_destroy(blocks);
	return;
}
