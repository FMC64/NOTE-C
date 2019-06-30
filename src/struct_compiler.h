
#pragma once

typedef struct {
	size_t count;
	size_t allocated;
	char **str;
} VecStr;

typedef struct {
	VecStr tokens;
	int input_file;
} CBuf;

typedef struct {
	CBuf buf;
} CParser;
