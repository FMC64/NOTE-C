
#pragma once

typedef struct {
	void *ptr;
	Context ctx;
} MemcheckBlock;

typedef struct {
	size_t count;
	size_t allocated;
	MemcheckBlock *block;
} VecMemcheckBlock;
