
#pragma once

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(value, min, max) (MAX(MIN((value), (max)), (min)))

#define ptr_add(a, b) ((void*)&(((char*)(a))[(size_t)(b)]))
