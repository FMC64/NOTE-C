
#pragma once

typedef struct {
	size_t size;
	char *data;
} Str;

typedef struct {
	size_t count;
	size_t allocated;
	char **str;
} VecStr;

typedef struct StrSonicNode StrSonicNode;

typedef struct {
	size_t count;
	StrSonicNode *node;
} VecStrSonicNode;

struct StrSonicNode {
	VecStrSonicNode sub;
	char *key;
	unsigned char type;
	void *value;
};

typedef struct {
	StrSonicNode node;
	void (*elem_destroy_cb)(unsigned char, void*);
} StrSonic;
