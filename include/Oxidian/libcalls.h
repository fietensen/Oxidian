#ifndef LIBCALLS_H
#define LIBCALLS_H

#include <Oxidian/environment.h>
#include <stdbool.h>

typedef struct {
    char *name; // name of the call
    char *description; // a brief description on what the function does
    unsigned int (*function)(OEnvironment*); // the function that's being invoked
} OCall;

typedef struct {
    char *name; // name of the library
    char *author; // author of the library
    char *description; // a brief description on what the library does
    unsigned short ncalls; // number of calls available (0 - USHORT_MAX) 

    OCall *calls;
} OLibrary;

#endif