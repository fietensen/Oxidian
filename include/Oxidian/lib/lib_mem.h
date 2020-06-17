#ifndef LIB_MEM_H
#define LIB_MEM_H
#include <Oxidian/libcalls.h>
#include <Oxidian/errors.h>
#include <Oxidian/helper_functions.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#define LIB_MEM_LNAME "MEMORY Library"
#define LIB_MEM_LAUTHOR "Fiete Minge"
#define LIB_MEM_LDESCRIPTION "Library for memory management"
#define LIB_MEM_LNCALLS 4

extern OCall lib_mem_calls[LIB_MEM_LNCALLS];

#endif
