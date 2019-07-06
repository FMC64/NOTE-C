
#pragma once

typedef struct {
	void *ptr;
	size_t size;
	Context ctx;
} MemcheckBlock;

typedef struct {
	size_t count;
	size_t allocated;
	MemcheckBlock *block;
} VecMemcheckBlock;
