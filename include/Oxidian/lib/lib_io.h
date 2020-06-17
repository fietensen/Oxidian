#ifndef LIB_IO_H
#define LIB_IO_H
#include <Oxidian/libcalls.h>
#include <Oxidian/errors.h>
#include <Oxidian/helper_functions.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#define LIB_IO_LNAME "IO Library"
#define LIB_IO_LAUTHOR "Fiete Minge"
#define LIB_IO_LDESCRIPTION "Provides several functions for input/output"
#define LIB_IO_LNCALLS 10

#define O_READ 0x0
#define O_WRITE 0x1
#define O_APP 0x2
#define O_READWRITE 0x3
#define O_CREAT 0x4

extern OCall lib_io_calls[LIB_IO_LNCALLS];

#endif
