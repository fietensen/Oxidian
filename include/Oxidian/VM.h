#ifndef VM_H
#define VM_H

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Oxidian/environment.h>
#include <Oxidian/instruction.h>
#include <Oxidian/helper_functions.h>
#include <Oxidian/functions.h>
#include <Oxidian/errors.h>
#include <Oxidian/config.h>

#if NETLIB
    #include <Oxidian/lib/lib_net.h>
#endif

int init_oxy(char *, int, char**, char**);

#endif
