#ifndef ERRORS_H
#define ERRORS_H

#include <stdbool.h>
#include <Oxidian/environment.h>

#define ERR_UNKNOWN 0
#define ERR_NOINST 1
#define ERR_NOLIB 2
#define ERR_NOCALL 3
#define ERR_STACKSPACE 4
#define ERR_INVALIDFMODE 5
#define ERR_NOFILE 6
#define ERR_FOPENERR 7
#define ERR_SEEK 8
#define ERR_FCLOSE 9
#define ERR_NOSOCK 10
#define ERR_SOCKET 11
#define ERR_INVALIDSOCK 12
#define ERR_CONNECT 13
#define ERR_SOCKSEND 14
#define ERR_SOCKCLOSE 15
#define ERR_RECV 16
#define ERR_GETHOSTBYNAME 17
#define ERR_SETSOCKOPT 18
#define ERR_LISTEN 19
#define ERR_BIND 20
#define ERR_SOCKACCEPT 21
#define ERR_SUCCESS 1337

// returning true if error is fatal and file execution must be stopped, otherwise false

bool runtimeerror(unsigned int error_n, OEnvironment*);

#endif
