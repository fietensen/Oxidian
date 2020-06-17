#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <stdbool.h>
#include <Oxidian/errors.h>
#include <Oxidian/config.h>
#include <Oxidian/libcalls.h>

#include <Oxidian/lib/lib_empty.h>
#ifdef IOLIB
    #include <Oxidian/lib/lib_io.h>
#endif
#ifdef MEMLIB
    #include <Oxidian/lib/lib_mem.h>
#endif
#ifdef NETLIB
    #include <Oxidian/lib/lib_net.h>
#endif

#define LIBS 3

bool handle_call(OEnvironment*);

#endif