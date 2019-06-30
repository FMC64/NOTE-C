
#pragma once

typedef struct {
	size_t count;
	size_t allocated;
	char **str;
} VecStr;

typedef struct {
	char *input_file_path;
	int input_file;
	VecStr tokens;
} CBuf;

typedef struct {
	CBuf buf;
} CParser;
