
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
