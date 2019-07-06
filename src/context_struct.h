
#pragma once

typedef struct {
	const char *file;
	int line;
} Context;

typedef struct {
	const char *file;
	int line;
	int colon;
} CContext;
