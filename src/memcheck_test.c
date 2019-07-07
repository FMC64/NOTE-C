
#include "headers.h"

void memcheck_test_rnd_alloc(void)
{
	void *ptr[16];
	size_t i;
	size_t j;

	for (i = 0; i < 100; i++) {
		for (j = 0; j < 16; j++)
			ptr[j] = malloc(rand() % 128);
		for (j = 0; j < 16; j++)
			free(ptr[j]);
	}
}
