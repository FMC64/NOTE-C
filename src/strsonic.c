
#include "headers.h"

static VecStrSonicNode VecStrSonicNode_init(void)
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
}

static StrSonicNode StrSonicNode_init(const char *key, unsigned char type, void *value)
{
	StrSonicNode res;

	res.sub = VecStrSonicNode_init();
	res.key = key;
	res.type = type;
	res.value = value;
	return res;
}

StrSonicNode StrSonicNode_null(void)
{
	return StrSonicNode_init(NULL, 0, NULL);
}


StrSonic StrSonic_init(void (*elem_destroy_cb)(unsigned char, void*))
{
	StrSonic res;

	res.node = StrSonicNode_null();
	res.elem_destroy_cb = elem_destroy_cb;
	return res;
}

static void StrSonicNode_split(StrSonicNode *node, size_t len, StrSonicNode to_share)
{
	StrSonicNode new_node = StrSonicNode_null();
	char *sub_key = strdup(&node->key[len]);
	char *new_key;

	new_node.key = node->key;
	new_node.key[len] = 0;
	new_key = strdup(new_node.key);
	free(new_node.key);
	new_node.key = new_key;

	node->key = sub_key;
	VecStrSonicNode_add(&new_node.sub, *node);
	VecStrSonicNode_add(&new_node.sub, to_share);
	*node = new_node;
}

int StrSonic_add(StrSonic *sonic, const char *key, unsigned char type, void *value)
{
	StrSonicNode *node = &sonic->node;
	StrSonicNode *max;
	size_t max_value;
	size_t cur;
	size_t i;
	char *key_ac = key;

	while (1) {
		loop_start:

		if (key_ac[0] == 0) {
			if (node->value != NULL)
				return 0;
			node->type = type;
			node->value = value;
			if (node->key == NULL)
				node->key = strdup(key_ac);
			return 1;
		}
		max = NULL;
		max_value = 0;
		for (i = 0; i < node->sub.count; i++) {
			if (streq_part_max(key_ac, node->sub.node[i].key, &cur)) {
				node = &node->sub.node[i];
				key_ac = &key_ac[strlen(node->key)];
				goto loop_start;
			}
			if (cur > max_value) {
				max = &node->sub.node[i];
				max_value = cur;
			}
		}
		if (max != NULL) {
			StrSonicNode_split(max, max_value, StrSonicNode_init(strdup(&key_ac[max_value]), type, value));
			return 1;
		} else {
			VecStrSonicNode_add(&node->sub, StrSonicNode_init(strdup(key_ac), type, value));
			return 1;
		}
	}
}

static void StrSonicNode_print_iter(StrSonicNode node, size_t depth)
{
	size_t i;

	for (i = 0; i < depth; i++)
		printf("  ");
	printf("'%s': %p\n", node.key != NULL ? node.key : "NULL", node.value);
	for (i = 0; i < node.sub.count; i++)
		StrSonicNode_print_iter(node.sub.node[i], depth + 1);
}

void StrSonic_print(StrSonic sonic)
{
	terminal_flush();
	StrSonicNode_print_iter(sonic.node, 0);
	terminal_show();
}

void* StrSonic_resolve(StrSonic *sonic, const char *key, unsigned char *type)
{
	StrSonicNode *node = &sonic->node;
	size_t i;
	char *key_ac = key;

	while (1) {
		loop_start:

		if (key_ac[0] == 0)
			return node->value;
		for (i = 0; i < node->sub.count; i++) {
			if (streq_part(key_ac, node->sub.node[i].key)) {
				node = &node->sub.node[i];
				key_ac = &key_ac[strlen(node->key)];
				goto loop_start;
			}
		}
		return NULL;
	}
}

static void destroy_elem(StrSonic *sonic, void **data)
{
	if (sonic->elem_destroy_cb != NULL)
		if (*data != NULL) {
			sonic->elem_destroy_cb(0, *data);
			*data = NULL;
		}
}

void StrSonic_destroy_elem(StrSonic *sonic, const char *key)
{
	//destroy_elem(sonic, StrSonic_resolve_ref(sonic, key));
}

static void StrSonicNode_destroy_iter(StrSonic *sonic, StrSonicNode *node, void (*elem_destroy_cb)(unsigned char, void*))
{
	size_t i;

	for (i = 0; i < node->sub.count; i++)
		StrSonicNode_destroy_iter(sonic, &node->sub.node[i], elem_destroy_cb);
	VecStrSonicNode_destroy(node->sub);
	free(node->key);
	destroy_elem(sonic, &node->value);
}

void StrSonic_destroy(StrSonic *sonic)
{
	StrSonicNode_destroy_iter(sonic, &sonic->node, sonic->elem_destroy_cb);
}
