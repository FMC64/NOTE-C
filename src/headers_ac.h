
#pragma once


#define SH4


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#ifndef _SSIZE_T
#define _SSIZE_T
typedef signed long ssize_t;
#endif
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>

#ifndef M_PI
#define M_PI (float)3.14159265358979323846
#endif

#include <fxlib.h>
#ifdef SH4
#include "sh4.h"
#endif

#include "MonochromeLib.h"

#include "struct.h"

#include "malloc.h"
#include "mat.h"
#include "context.h"
#include "memcheck.h"
#include "string.h"
#include "io.h"
#include "compiler.h"
#include "terminal.h"
#include "main.h"
#include "macro.h"
