
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

#define STRSONIC_SLOW

/* STRSONIC TREE STRUCTURE

FMC64:
I've put the whole StrSonicNode and all attributes in a single buffer (to avoid padding and malloc overhead)
Stuff is ordered in a certain way, with the key in plain buffer. That's actually a stream.

ORDER:
unsigned char: type, 2 bits flag: 0x80 is subCount == 0 (no subCount if toggled), 0x40 is toggled if no data is set (no data either)
str: key (just a basic, null terminated C string)
size_t: subCount
void**: sub
void*: data

*/

typedef struct {
	unsigned char type;
	char *key;		// Safe to read
	size_t subCount;
	void **sub;		// /!\ Danger /!\ - Do not read directly (unaligned pointer), use macro cpy() instead
	void *data;		// Safe (copied on dump)
} StrSonicNode;		// Uncompressed version of above data, use StrSonicNode_dump to get this

typedef struct {
	size_t count;
	size_t allocated;
	StrSonicNode *node;
} VecStrSonicNode;

typedef struct {
	#ifndef STRSONIC_SLOW
	void *root;
	#else
	VecStrSonicNode nodes;
	#endif
	void (*elem_destroy_cb)(unsigned char, void*);
} StrSonic;
