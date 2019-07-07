
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

	#ifdef MEMCHECK_LIGHT
	res.count = 0;
	res.allocated = 0;
	res.block = NULL;

	#else
	res.node = NULL;

	#endif // MEMCHECK_LIGHT
	return res;
}

static MemcheckBlockNode MemcheckBlockNode_init(MemcheckBlockNode *root)
{
	MemcheckBlockNode res;
	size_t i;

	res.root = root;
	for (i = 0; i < 2; i++)
		res.sub[i] = NULL;
	res.block = NULL;
	return res;
}

static MemcheckBlockNode* MemcheckBlockNode_alloc(MemcheckBlockNode *root)
{
	MemcheckBlockNode *res = (MemcheckBlockNode*)malloc(sizeof(MemcheckBlockNode));

	*res = MemcheckBlockNode_init(root);
	return res;
}

static void MemcheckBlockNode_node_count_iter(MemcheckBlockNode *node, size_t *pacc)
{
	size_t i;

	if (node == NULL)
		return;
	(*pacc)++;
	for (i = 0; i < 2; i++)
		MemcheckBlockNode_node_count_iter(node->sub[i], pacc);
}

static size_t MemcheckBlockNode_node_count(MemcheckBlockNode *node)
{
	size_t res = 0;

	MemcheckBlockNode_node_count_iter(node, &res);
	return res;
}

static void VecMemcheckBlock_add(VecMemcheckBlock *vec, MemcheckBlock to_add)
{
	#ifdef MEMCHECK_LIGHT
	size_t cur;

	if (to_add.ptr == NULL)
		return;
	cur = vec->count++;
	if (vec->count > vec->allocated) {
		vec->allocated += 16;
		vec->block = (MemcheckBlock*)realloc(vec->block, vec->allocated * sizeof(MemcheckBlock));
	}
	vec->block[cur] = to_add;

	#else
	// Binary tree node insertion for radix (base 2) search

	MemcheckBlockNode *root = NULL;
	MemcheckBlockNode **cur = &vec->node;
	size_t i;
	size_t key = (size_t)to_add.ptr;
	size_t key_bits = sizeof(key) * 8;

	if (to_add.ptr == NULL)
		return;
	if ((*cur) == NULL)
		*cur = MemcheckBlockNode_alloc(root);
	for (i = 0; i < key_bits; i++) {
		root = *cur;
		cur = &(*cur)->sub[(key >> (key_bits - 1 - i)) & 1];
		if ((*cur) == NULL)
			*cur = MemcheckBlockNode_alloc(root);
	}

	if ((*cur)->block != NULL) {
		terminal_flush();

		printf_term("malloc error:\nduplicated block %p\n", to_add.ptr);

		terminal_show();
		exit(0);
	}

	(*cur)->block = (MemcheckBlock*)malloc(sizeof(MemcheckBlock));
	*(*cur)->block = to_add;

	#endif // MEMCHECK_LIGHT
}

static void MemcheckBlockNode_count_iter(MemcheckBlockNode *node, size_t *pacc)
{
	size_t i;

	if (node == NULL)
		return;
	for (i = 0; i < 2; i++)
		MemcheckBlockNode_count_iter(node->sub[i], pacc);
	if (node->block != NULL)
		(*pacc)++;
}

static size_t VecMemcheckBlock_block_count(VecMemcheckBlock vec)
{
	#ifdef MEMCHECK_LIGHT
	return vec.count;

	#else
	size_t res = 0;

	MemcheckBlockNode_count_iter(vec.node, &res);
	return res;

	#endif
}

static void MemcheckBlockNode_size_iter(MemcheckBlockNode *node, size_t *pacc)
{
	size_t i;

	if (node == NULL)
		return;
	for (i = 0; i < 2; i++)
		MemcheckBlockNode_size_iter(node->sub[i], pacc);
	if (node->block != NULL)
		(*pacc) += node->block->size;
}

static size_t VecMemcheckBlock_blocks_size(VecMemcheckBlock vec)
{
	#ifdef MEMCHECK_LIGHT
	size_t res = 0;
	size_t i;

	for (i = 0; i < vec.count; i++)
		res += vec.block[i].size;
	return res;

	#else
	size_t res = 0;

	MemcheckBlockNode_size_iter(vec.node, &res);
	return res;

	#endif
}

#ifdef MEMCHECK_LIGHT
static void VecMemcheckBlock_remove(VecMemcheckBlock *vec, size_t ndx)
{
	vec->count--;
	if (vec->count > 0)
		vec->block[ndx] = vec->block[vec->count];
}
#endif

static void MemcheckBlockNode_free_retro(MemcheckBlockNode *to_free)
{
	MemcheckBlockNode *root;
	size_t i;

	if (to_free == NULL)
		return;
	root = to_free->root;
	free(to_free->block);
	to_free->block = NULL;

	// Free node and all its references if empty
	for (i = 0; i < 2; i++)
		if (to_free->sub[i] != NULL)
			return;
	if (root != NULL)
		for (i = 0; i < 2; i++)
			if (root->sub[i] == to_free)
				root->sub[i] = NULL;
	free(to_free);
	// Retro-action
	MemcheckBlockNode_free_retro(root);
}

static int VecMemcheckBlock_search_delete(VecMemcheckBlock *vec, void *ptr)
{
	#ifdef MEMCHECK_LIGHT
	size_t i;

	if (ptr == NULL)
		return 1;
	for (i = 0; i < vec->count; i++) {
		if (vec->block[i].ptr == ptr) {
			VecMemcheckBlock_remove(vec, i);
			return 1;
		}
	}
	return 0;

	#else
	MemcheckBlockNode **cur = &vec->node;
	size_t i;
	size_t key = (size_t)ptr;
	size_t key_bits = sizeof(key) * 8;

	// Binary tree radix (base 2) search
	if (ptr == NULL)
		return 1;
	if ((*cur) == NULL)
		return 0;
	for (i = 0; i < key_bits; i++) {
		cur = &(*cur)->sub[(key >> (key_bits - 1 - i)) & 1];
		if ((*cur) == NULL)
			return 0;
	}
	MemcheckBlockNode_free_retro(*cur);
	return 1;
	#endif // MEMCHECK_LIGHT
}

static void VecMemcheckBlock_free(VecMemcheckBlock *vec, void *ptr, Context ctx)
{
	if (VecMemcheckBlock_search_delete(vec, ptr))
		return;

	terminal_flush();

	printf_term("malloc error: block not found\n");
	printf_term("Can't free %p\n", ptr);
	Context_print_term(ctx);
	printf_term("%u blocks allocated\n\n", VecMemcheckBlock_block_count(*vec));
	printf_term("Press EXIT to continue");

	terminal_show();
	exit(0);
}

static void VecMemcheckBlock_destroy(VecMemcheckBlock blocks)
{
	#ifdef MEMCHECK_LIGHT
	free(blocks.block);

	#else
	// la flemme

	#endif // MEMCHECK_LIGHT
	return;
}

static VecMemcheckBlock blocks = VECMEMCHECKBLOCK_INIT;
static is_disabled = 0;

static void alloc_check(size_t size, void *ptr, Context ctx)
{
	if (!((ptr == NULL) && (size > 0)))
		return;

	terminal_flush();

	printf_term("malloc error: block can't be allocated\n");
	printf_term("Returned null for %u bytes\n", size);
	Context_print_term(ctx);
	printf_term("net %u blocks allocated\ntotal %u bytes\n\n", VecMemcheckBlock_block_count(blocks), VecMemcheckBlock_blocks_size(blocks));
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

static void MemcheckBlock_print_iter(MemcheckBlockNode *node)
{
	size_t i;

	if (node == NULL)
		return;
	if (node->block != NULL) {
		MemcheckBlock_print(*node->block);
		printf_term("\n");
	}
	for (i = 0; i < 2; i++)
		MemcheckBlock_print_iter(node->sub[i]);
}

static void VecMemcheckBlock_print(VecMemcheckBlock vec)
{
	#ifdef MEMCHECK_LIGHT
	size_t i;

	for (i = 0; i < vec.count; i++) {
		MemcheckBlock_print(vec.block[i]);
		printf_term("\n");
	}

	#else
	MemcheckBlock_print_iter(vec.node);

	#endif // MEMCHECK_LIGHT
}

void memcheck_recap(void)
{
	size_t block_count;
	size_t blocks_size;

	terminal_flush();
	is_disabled = 1;
	block_count = VecMemcheckBlock_block_count(blocks);
	blocks_size = VecMemcheckBlock_blocks_size(blocks);
	if (block_count == 0)
		printf_term("malloc OK\n\n");
	else {
		printf_term("malloc error:\n");
		printf_term("net %u blocks allocated\ntotal %u bytes\n\n", block_count, blocks_size);
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
	size_t block_count = VecMemcheckBlock_block_count(blocks);
	size_t total = VecMemcheckBlock_blocks_size(blocks);
	size_t overhead = block_count * 4;
	#ifdef MEMCHECK_LIGHT
	size_t memcheck_overhead = block_count * sizeof(MemcheckBlock);
	#else
	size_t memcheck_overhead = MemcheckBlockNode_node_count(blocks.node) * (sizeof(MemcheckBlockNode) + 4) + block_count * (sizeof(MemcheckBlock) + 4);
	#endif // MEMCHECK_LIGHT
	terminal_flush();

	printf_term("%u blocks\n%u bytes allocated\noverhead: %u bytes\nmemcheck overhead: %u bytes\n\nTOTAL: %u bytes\n",
	block_count, total, overhead, memcheck_overhead, total + overhead + memcheck_overhead);

	terminal_show();
}
