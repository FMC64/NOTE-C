
#include "headers.h"

/*static VecStrSonicNode VecStrSonicNode_init(void)
{
	VecStrSonicNode res;

	res.count = 0;
	res.node = NULL;
	return res;
}

static void VecStrSonicNode_add(VecStrSonicNode *vec, StrSonicNode to_add)
{
	size_t cur = vec->count++;

	vec->node = (StrSonicNode*)realloc(vec->node, vec->count * sizeof(StrSonicNode));
	vec->node[cur] = to_add;
}

static void VecStrSonicNode_destroy(VecStrSonicNode vec)
{
	free(vec.node);
}*/

#define NODE_TYPE_SUBCOUNTZERO 0x80
#define NODE_TYPE_NODATA 0x40

static StrSonicNode StrSonicNode_init(size_t type, char *key, size_t subCount, void **sub, void *data)
{
	StrSonicNode res;

	res.type = type;
	res.key = key;
	res.subCount = subCount;
	res.sub = sub;
	res.data = data;
	return res;
}

static StrSonicNode StrSonicNode_dump(void **pdata)
{
	StrSonicNode res;
	void *data;
	size_t i = 0;

	cpy(data, *pdata);
	res.type = *(unsigned char*)data;
	i += sizeof(unsigned char);
	res.key = (char*)ptr_add(data, i);
	i += strlen(res.key) + 1;
	if (res.type & NODE_TYPE_SUBCOUNTZERO) {
		res.subCount = 0;
		res.sub = NULL;
	} else {
		cpy(res.subCount, *(size_t*)ptr_add(data, i));
		i += sizeof(size_t);
		res.sub = (void**)ptr_add(data, i);
		i += res.subCount * sizeof(void*);
	}
	if (res.type & NODE_TYPE_NODATA)
		res.data = NULL;
	else
		cpy(res.data, *(void**)ptr_add(data, i));
	res.type &= ~(NODE_TYPE_NODATA | NODE_TYPE_SUBCOUNTZERO);
	return res;
}

static size_t StrSonicNode_size(StrSonicNode *node)
{
	size_t res = 1;

	res += strlen(node->key) + 1;
	if (node->subCount > 0)
		res += sizeof(size_t) + (node->subCount * sizeof(void*));
	if (node->data != NULL)
		res += sizeof(void*);
	return res;
}

static void* StrSonicNode_create_adv(StrSonicNode *node, size_t subToCopy)
{
	void *res = malloc(StrSonicNode_size(node));
	unsigned char type = node->type & ~(NODE_TYPE_NODATA | NODE_TYPE_SUBCOUNTZERO);
	size_t i = 0;

	*(unsigned char*)res = 0;
	i += sizeof(unsigned char);
	strcpy((char*)ptr_add(res, i), node->key);
	i += strlen(node->key) + 1;
	if (node->subCount > 0) {
		cpy(*(size_t*)ptr_add(res, i), node->subCount);
		i += sizeof(size_t);
		memcpy((void**)ptr_add(res, i), node->sub, subToCopy * sizeof(void*));
		i += node->subCount * sizeof(void*);
	} else
		type |= NODE_TYPE_SUBCOUNTZERO;
	if (node->data != NULL)
		cpy(*(void**)ptr_add(res, i), node->data);
	else
		type |= NODE_TYPE_NODATA;
	*(unsigned char*)res = type;
	return res;
}

static void* StrSonicNode_create(StrSonicNode *node)
{
	return StrSonicNode_create_adv(node, node->subCount);
}

static void* StrSonicNode_createNoRef(StrSonicNode node)
{
	return StrSonicNode_create(&node);
}

void* StrSonicNode_null(void)
{
	return StrSonicNode_createNoRef(StrSonicNode_init(0, "", 0, NULL, NULL));
}

StrSonic StrSonic_init(void (*elem_destroy_cb)(unsigned char, void*))
{
	StrSonic res;

	res.root = StrSonicNode_null();
	res.elem_destroy_cb = elem_destroy_cb;
	return res;
}

static void* StrSonicNode_cloneNewKey(void *node, char *new_key)
{
	StrSonicNode new_node = StrSonicNode_dump(&node);

	new_node.key = new_key;
	return StrSonicNode_create(&new_node);
}

static void StrSonicNode_split(void **pnode, size_t len, void *to_share)
{
	void *node;
	void *new_node;
	char *new_key;
	void *sub_node;
	StrSonicNode snode = StrSonicNode_dump(pnode);

	cpy(node, *pnode);
	sub_node = StrSonicNode_cloneNewKey(node, &snode.key[len]);

	new_key = strdup(snode.key);
	new_key[len] = 0;
	snode = StrSonicNode_init(0, new_key, 2, NULL, NULL);
	new_node = StrSonicNode_create_adv(&snode, 0);
	snode = StrSonicNode_dump(&new_node);
	free(new_key);

	cpy(snode.sub[0], sub_node);
	cpy(snode.sub[1], to_share);
	free(node);
	cpy(*pnode, new_node);
}

static void StrSonicNode_subAdd(void **pnode, void *to_add)
{
	StrSonicNode snode = StrSonicNode_dump(pnode);
	void *node;
	void *new_node;

	cpy(node, *pnode);
	snode.subCount++;
	new_node = StrSonicNode_create_adv(&snode, snode.subCount - 1);
	snode = StrSonicNode_dump(&new_node);
	cpy(snode.sub[snode.subCount - 1], to_add);
	free(node);
	cpy(*pnode, new_node);
}

static char* StrSonicNode_key(void *data)
{
	return (char*)ptr_add(data, 1);
}

static void StrSonicNode_setData(void **pnode, unsigned char type, void *data)
{
	StrSonicNode snode = StrSonicNode_dump(pnode);
	void *node;
	void *new_node;

	cpy(node, *pnode);
	snode.type = type;
	snode.data = data;
	free(node);
	new_node = StrSonicNode_create(&snode);
	cpy(*pnode, new_node);
}

int StrSonic_add(StrSonic *sonic, const char *key, unsigned char type, void *data)
{
	void **pnode = &sonic->root;
	StrSonicNode cur;
	void **psub;
	void *sub;
	void **max;
	size_t max_value;
	size_t cur_value;
	size_t i;
	char *key_ac = key;

	cur = StrSonicNode_dump(pnode);
	while (1) {
		loop_start:

		if (key_ac[0] == 0) {
			if (cur.data != NULL)
				return 0;
			StrSonicNode_setData(pnode, type, data);
			return 1;
		}
		max = NULL;
		max_value = 0;
		for (i = 0; i < cur.subCount; i++) {
			psub = &cur.sub[i];
			cpy(sub, *psub);
			if (streq_part_max(key_ac, StrSonicNode_key(sub), &cur_value)) {
				pnode = psub;
				cur = StrSonicNode_dump(pnode);
				key_ac = &key_ac[strlen(StrSonicNode_key(sub))];
				goto loop_start;
			}
			if (cur_value > max_value) {
				max = psub;
				max_value = cur_value;
			}
		}
		if (max != NULL) {
			StrSonicNode_split(max, max_value, StrSonicNode_createNoRef(StrSonicNode_init(type, &key_ac[max_value], 0, NULL, data)));
			return 1;
		} else {
			StrSonicNode_subAdd(pnode, StrSonicNode_createNoRef(StrSonicNode_init(type, key_ac, 0, NULL, data)));
			return 1;
		}
	}
}

static void StrSonicNode_print_iter(void *node, size_t depth)
{
	StrSonicNode snode = StrSonicNode_dump(&node);
	void *sub;
	size_t i;

	for (i = 0; i < depth; i++)
		printf("  ");
	printf("'%s': %p\n", snode.key, snode.data);
	for (i = 0; i < snode.subCount; i++) {
		cpy(sub, snode.sub[i]);
		StrSonicNode_print_iter(sub, depth + 1);
	}
}

void StrSonic_print(StrSonic sonic)
{
	terminal_flush();
	StrSonicNode_print_iter(sonic.root, 0);
	terminal_show();
}

static int StrSonic_resolveNode(StrSonic *sonic, const char *key, void ***pres)
{
	void **pnode = &sonic->root;
	StrSonicNode cur;
	void **psub;
	void *sub;
	size_t i;
	char *key_ac = key;

	cur = StrSonicNode_dump(pnode);
	while (1) {
		loop_start:

		if (key_ac[0] == 0) {
			*pres = pnode;
			return 1;
		}
		for (i = 0; i < cur.subCount; i++) {
			psub = &cur.sub[i];
			cpy(sub, *psub);
			if (streq_part(key_ac, StrSonicNode_key(sub))) {
				pnode = psub;
				cur = StrSonicNode_dump(pnode);
				key_ac = &key_ac[strlen(StrSonicNode_key(sub))];
				goto loop_start;
			}
		}
		return 0;
	}
}

void* StrSonic_resolve(StrSonic *sonic, const char *key, unsigned char *type)
{
	void **pnode;
	StrSonicNode snode;

	if (!StrSonic_resolveNode(sonic, key, &pnode))
		return NULL;
	snode = StrSonicNode_dump(pnode);
	*type = snode.type;
	return snode.data;
}

static void destroy_node_elem(StrSonic *sonic, void **pnode)
{
	void *node;
	void *new_node;
	StrSonicNode snode;

	snode = StrSonicNode_dump(pnode);
	if (sonic->elem_destroy_cb != NULL)
		sonic->elem_destroy_cb(snode.type, snode.data);
	snode.type = 0;
	snode.data = NULL;
	new_node = StrSonicNode_create(&snode);
	cpy(node, *pnode);
	free(node);
	cpy(*pnode, new_node);
}

static void destroy_elem(StrSonic *sonic, unsigned char type, void *data)
{
	if (sonic->elem_destroy_cb != NULL)
		sonic->elem_destroy_cb(type, data);
}

void StrSonic_destroy_elem(StrSonic *sonic, const char *key)
{
	void **pnode;

	if (!StrSonic_resolveNode(sonic, key, &pnode))
		return;
	destroy_node_elem(sonic, pnode);
}

static void StrSonicNode_destroy_iter(StrSonic *sonic, void **pnode)
{
	StrSonicNode cur = StrSonicNode_dump(pnode);
	void *node;
	size_t i;

	for (i = 0; i < cur.subCount; i++)
		StrSonicNode_destroy_iter(sonic, &cur.sub[i]);
	destroy_elem(sonic, cur.type, cur.data);
	cpy(node, *pnode);
	free(node);
}

void StrSonic_destroy(StrSonic *sonic)
{
	StrSonicNode_destroy_iter(sonic, &sonic->root);
}
