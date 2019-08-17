
#pragma once

// Set this macro to enable light memory checking (slowest but less memory consuming)
// You should leave this defined in general
#define MEMCHECK_LIGHT

typedef struct {
	void *ptr;
	size_t size;
	Context ctx;
} MemcheckBlock;

typedef struct MemcheckBlockNode MemcheckBlockNode;
struct MemcheckBlockNode {
	MemcheckBlockNode *root;
	MemcheckBlockNode *sub[2];
	MemcheckBlock *block;
};

typedef struct MemcheckBlockLink MemcheckBlockLink;
struct MemcheckBlockLink {
	MemcheckBlock block;
	MemcheckBlockLink *next;
};

typedef struct {
	#ifdef MEMCHECK_LIGHT
	MemcheckBlockLink **last;
	MemcheckBlockLink *next;

	#else
	MemcheckBlockNode *node;

	#endif // MEMCHECK_LIGHT
} VecMemcheckBlock;

#ifdef MEMCHECK_LIGHT
#define VECMEMCHECKBLOCK_INIT {&.next, NULL}

#else
#define VECMEMCHECKBLOCK_INIT {NULL}

#endif // MEMCHECK_LIGHT
