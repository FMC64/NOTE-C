
#pragma once

static Context Context_init(const char *file, int line)
{
	Context res;

	res.file = file;
	res.line = line;
	return res;
}

#define Context_build Context_init(__FILE__, __LINE__)

void Context_print(Context ctx, int *y);
