
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

void memcheck_test_limit(void)
{
	size_t blk_size = 4000;
	char *ptr;
	size_t i;
	size_t j;

	for (i = 0; i < 10000; i++) {
		ptr = malloc(blk_size);
		for (j = 0; j < blk_size; j++)
			ptr[j] = (char)(i + 0x7F);
		for (j = 0; j < blk_size; j++)
			if (ptr[j] != (char)(i + 0x7F)) {
				terminal_flush();

				printf_term("Iteration #%u, byte %u:\nIncoherent heap.\n", i, j);

				terminal_show();
				exit(0);
			}
	}
}

void memcheck_test_fragmenting(void)
{
	size_t blk_size;
	char *ptr[64];
	size_t i;
	size_t j;
	size_t k;

	for (k = 0; k < 100; k++) {
		for (i = 0; i < 64; i++) {
			blk_size = rand() % 4000;
			ptr[i] = malloc(blk_size);
			for (j = 0; j < blk_size; j++)
				ptr[i][j] = (char)(i + 0x7F);
			for (j = 0; j < blk_size; j++)
				if (ptr[i][j] != (char)(i + 0x7F)) {
					terminal_flush();

					printf_term("Iteration #%u, byte %u:\nIncoherent heap.\n", i, j);

					terminal_show();
					exit(0);
				}
		}
		for (i = 0; i < 64; i++)
			free(ptr[i]);
	}
}
