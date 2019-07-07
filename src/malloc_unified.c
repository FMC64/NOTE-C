
#include <stdlib.h>

#include "macro.h"

void *malloc_ext(unsigned int size);
void *realloc_ext(void *ptr, size_t size);
void free_ext(void *ptr);

// Thanks to Sentaro21 for this
static void * HiddenRAM(void){    // Check HiddenRAM
	volatile unsigned int *NorRAM = (volatile unsigned int*)0xA8000000;    // Nomarl RAM TOP (no cache area)
	volatile unsigned int *HidRAM = (volatile unsigned int*)0x88040000;    // Hidden RAM TOP (cache area)
	int a, b;
	int K55 = 0x55555555;
	int KAA = 0xAAAAAAAA;
	char *HidAddress=NULL;

	a = *NorRAM;
	b = *HidRAM;
	*NorRAM = K55;
	*HidRAM = KAA;
	if ( *NorRAM != *HidRAM ) {
		HidAddress = (char*)HidRAM;    // Hidden RAM Exist
	}
	*NorRAM = a;
	*HidRAM = b;
	return HidAddress;
}

static void* ext_ram = NULL;
static size_t ext_ram_size = 0;

static void hidden_ram_test(void)
{
	size_t i;
	char *ptr = ext_ram;


	if (ptr == NULL) {
		terminal_flush();

		printf_term("No additional RAM found.\n");

		terminal_show();
		exit(0);
	}
	for (i = 0; i < ext_ram_size; i++)
		ptr[i] = 70;
	for (i = 0; i < ext_ram_size; i++)
		if (ptr[i] != 70) {
			terminal_flush();

			printf_term("Unreliable additional RAM.\n");

			terminal_show();
			exit(0);
		}
}

static int is_ext_available(void)
{
	return ext_ram != NULL;
}

static int is_block_ext(void *ptr)
{
	return (ptr >= ext_ram) && (ptr < (ptr_add(ext_ram, ext_ram_size)));
}

void malloc_unified_init(void)
{
	ext_ram = HiddenRAM();
	if (ext_ram != NULL) {
		ext_ram_size = 256000;
		__malloc_ext_init(ext_ram, ext_ram_size);
	}
}

void* malloc_unified(size_t size)
{
	void *ptr;

	if (size == 0)
		return NULL;
	if (is_ext_available()) {
		ptr = malloc_ext(size);
		if (ptr != NULL)
			return ptr;
	}
	return malloc(size);
}

void* calloc_unified(size_t member_count, size_t size)
{
	void *ptr = malloc_unified(member_count * size);

	memset(ptr, 0, size);
	return ptr;
}

void* realloc_unified(void *ptr, size_t size)
{
	if (is_block_ext(ptr))
		return realloc_ext(ptr, size);
	else
		return realloc(ptr, size);
}

void* free_unified(void *ptr)
{
	if (ptr == NULL)
		return;
	if (is_block_ext(ptr))
		free_ext(ptr);
	else
		free(ptr);
}
