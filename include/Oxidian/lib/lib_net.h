#ifndef LIB_NET_H
#define LIB_NET_H
#include <Oxidian/libcalls.h>
#include <Oxidian/errors.h>
#include <Oxidian/helper_functions.h>
#include <Oxidian/config.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#if NETLIB
#if OPENSSL
    #include <openssl/ssl.h>
#endif

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#define LIB_NET_LNAME "NETWORKING Library"
#define LIB_NET_LAUTHOR "Fiete Minge"
#define LIB_NET_LDESCRIPTION "Library for networking"
#define LIB_NET_LNCALLS 13

typedef struct {
    struct sockaddr_in sAddr;
#ifdef _WIN32
    SOCKET socket;
#else
    int socket;
#endif
    bool used;
    bool use_ssl;
#if OPENSSL
    SSL* connection;
#endif
} OSocket;

extern OCall lib_net_calls[LIB_NET_LNCALLS];
void init_networking();
void clean_networking();
#endif
#endif
