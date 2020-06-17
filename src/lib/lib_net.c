#include <Oxidian/lib/lib_net.h>

OSocket sockets[N_SOCKS];

#if OPENSSL
SSL_CTX* ssl_ctx;
void init_openssl() {
    SSL_load_error_strings();
    SSL_library_init();
    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
}
#endif

#ifdef _WIN32
WSADATA wsa;
void init_wsa() {
    WSAStartup(MAKEWORD(2,2),&wsa);
}
void exit_wsa() {
    WSACleanup();
}
#endif

void init_networking() {
#ifdef _WIN32
    init_wsa();
#endif
#if OPENSSL
    init_openssl();
#endif

    for (unsigned int i=0;i<N_SOCKS;i++) {
        sockets[i].use_ssl = false;
        sockets[i].used = false;
    }
}

void clean_networking() {
#ifdef _WIN32
    exit_wsa();
#endif
}

unsigned int get_free_socket(OEnvironment *Oenviron) {
    for (unsigned int i=0;i<N_SOCKS;i++) {
        if (!sockets[i].used) {
            return i;
        }
    }

    return -1;
}

unsigned int lib_net_csocket(OEnvironment *Oenviron) {
    unsigned int free_socket = get_free_socket(Oenviron);
    if (free_socket == -1) return ERR_NOSOCK;
    OSocket *sockobj = &sockets[free_socket];

    unsigned int domain = *(unsigned int*)Oenviron->arguments[0]->value;
    unsigned int type = *(unsigned int*)Oenviron->arguments[1]->value;
    unsigned int protocol = *(unsigned int*)Oenviron->arguments[2]->value;

    sockobj->socket = socket(domain, type, protocol);
#ifdef _WIN32
    if (sockobj->socket == INVALID_SOCKET) return ERR_SOCKET;
#else
    if (sockobj->socket == -1) return ERR_SOCKET;
#endif

    int_to_char(free_socket, (char*)Oenviron->arguments[3]->value);
    sockobj->used = true;
    return ERR_SUCCESS;
}

unsigned int lib_net_csconnect(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;

    sockets[sock_select].sAddr.sin_port = htons(*(unsigned int*)Oenviron->arguments[3]->value);
    sockets[sock_select].sAddr.sin_family = *(unsigned int*)Oenviron->arguments[1]->value;
    sockets[sock_select].sAddr.sin_addr.s_addr = inet_addr((char*)Oenviron->arguments[2]->value);
#ifdef _WIN32
    if (connect(sockets[sock_select].socket, (struct sockaddr*)&sockets[sock_select].sAddr, sizeof(sockets[sock_select].sAddr)) == SOCKET_ERROR) return ERR_CONNECT;
#else
    if (connect(sockets[sock_select].socket, (struct sockaddr*)&sockets[sock_select].sAddr, sizeof(sockets[sock_select].sAddr)) == -1) return ERR_CONNECT;
#endif
#if OPENSSL
    if (sockets[sock_select].use_ssl) {
        sockets[sock_select].connection = SSL_new(ssl_ctx);
        SSL_set_fd(sockets[sock_select].connection, sockets[sock_select].socket);
        if (SSL_connect(sockets[sock_select].connection) != 1) return ERR_CONNECT;
    }
#endif
    return ERR_SUCCESS;
}

unsigned int lib_net_cssend(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;

#if OPENSSL
    if (sockets[sock_select].use_ssl) {
        if (SSL_write(sockets[sock_select].connection, (char*)Oenviron->arguments[1]->value, *(unsigned int*)Oenviron->arguments[2]->value) < 0) return ERR_SOCKSEND;
    }
#endif
    if (!(sockets[sock_select].use_ssl)) {
#ifdef _WIN32
        if (send(sockets[sock_select].socket, (char*)Oenviron->arguments[1]->value, *(unsigned int*)Oenviron->arguments[2]->value, *(unsigned int*)Oenviron->arguments[3]->value) == SOCKET_ERROR) return ERR_SOCKSEND;
#else
        if (send(sockets[sock_select].socket, (char*)Oenviron->arguments[1]->value, *(unsigned int*)Oenviron->arguments[2]->value, *(unsigned int*)Oenviron->arguments[3]->value) == -1) return ERR_SOCKSEND;
#endif
    }
    
    return ERR_SUCCESS;
}

unsigned int lib_net_csclose(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;

    sockets[sock_select].used = false;

#if OPENSSL
    if (sockets[sock_select].use_ssl) {
        SSL_shutdown(sockets[sock_select].connection);
        SSL_free(sockets[sock_select].connection);
    }
#endif
    sockets[sock_select].use_ssl = false;
#ifdef _WIN32
    if (closesocket(sockets[sock_select].socket) == SOCKET_ERROR) return ERR_SOCKCLOSE;
#else
    if (close(sockets[sock_select].socket) == -1) return ERR_SOCKCLOSE;
#endif

    return ERR_SUCCESS;
}

unsigned int lib_net_csrecv(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;

    unsigned int address = *(unsigned int*)Oenviron->arguments[2]->value;
    unsigned int length = *(unsigned int*)Oenviron->arguments[1]->value;
    unsigned int bytes_received = 0;

    if (Oenviron->stack_size-Oenviron->stackpointerposition < length) return ERR_STACKSPACE;

    if (!(sockets[sock_select].use_ssl)) {
#ifdef _WIN32
        if ((bytes_received = recv(sockets[sock_select].socket, (char*)&(Oenviron->stack[address]), length, 0)) == SOCKET_ERROR) return ERR_RECV;
#else
        if ((bytes_received = recv(sockets[sock_select].socket, &(Oenviron->stack[address]), length, 0)) == -1) return ERR_RECV;
#endif
    }
#if OPENSSL
    else {
        if ((bytes_received = SSL_read(sockets[sock_select].connection, Oenviron->stack+address, length)) < 0) return ERR_RECV;
    } 
#endif
    Oenviron->stackpointerposition += bytes_received;
    int_to_char(bytes_received, (char*)Oenviron->arguments[3]->value);
    return ERR_SUCCESS;
}

unsigned int lib_net_cgethostbyname(OEnvironment *Oenviron) {
    char* hostname = (char*)Oenviron->arguments[0]->value;
    char* ip = (char*)Oenviron->arguments[1]->value;
    struct hostent *he;
    if ((he = gethostbyname(hostname)) == NULL) return ERR_GETHOSTBYNAME;

    strcpy(ip, inet_ntoa(*((struct in_addr**)he->h_addr_list)[0]));

    return ERR_SUCCESS;
}

unsigned int lib_net_csetsockopt(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;

    unsigned int level = *(unsigned int*)Oenviron->arguments[1]->value;
    unsigned int optname = *(unsigned int*)Oenviron->arguments[2]->value;
    char *optval = (char*)Oenviron->arguments[3]->value;
    unsigned int optlen = *(unsigned int*)Oenviron->arguments[4]->value;

#ifdef _WIN32
    if (setsockopt(sockets[sock_select].socket, level, optname, optval, optlen) == SOCKET_ERROR) return ERR_SETSOCKOPT;
#else
    if (setsockopt(sockets[sock_select].socket, level, optname, optval, optlen) < 0) return ERR_SETSOCKOPT;
#endif

    return ERR_SUCCESS;
}

unsigned int lib_net_cslisten(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;

    unsigned int backlog = *(unsigned int*)Oenviron->arguments[1]->value;

#ifdef _WIN32
    if (listen(sockets[sock_select].socket, backlog) == SOCKET_ERROR) return ERR_LISTEN;
#else
    if (listen(sockets[sock_select].socket, backlog) < 0) return ERR_LISTEN;
#endif

    return ERR_SUCCESS;
}

unsigned int lib_net_csbind(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;
    
    sockets[sock_select].sAddr.sin_port = htons(*(unsigned int*)Oenviron->arguments[3]->value);
    sockets[sock_select].sAddr.sin_family = *(unsigned int*)Oenviron->arguments[1]->value;
    sockets[sock_select].sAddr.sin_addr.s_addr = inet_addr((char*)Oenviron->arguments[2]->value);
#ifdef _WIN32
    if (bind(sockets[sock_select].socket, (struct sockaddr*)&sockets[sock_select].sAddr, sizeof(sockets[sock_select].sAddr)) == SOCKET_ERROR) return ERR_BIND;
#else
    if (bind(sockets[sock_select].socket, (struct sockaddr*)&sockets[sock_select].sAddr, sizeof(sockets[sock_select].sAddr)) == -1) return ERR_BIND;
#endif

    return ERR_SUCCESS;
}

unsigned int lib_net_csaccept(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;
    unsigned int sock_client = get_free_socket(Oenviron);
    if (sock_client == -1) return ERR_NOSOCK;

    struct sockaddr* saddr = (struct sockaddr*)&sockets[sock_client].sAddr;
#ifdef _WIN32
    int sz = sizeof(struct sockaddr_in);
    if ((sockets[sock_client].socket = accept(sockets[sock_select].socket, saddr, &sz)) == SOCKET_ERROR) return ERR_SOCKACCEPT;
#else
    if ((sockets[sock_client].socket = accept(sockets[sock_select].socket, saddr, (socklen_t*)sizeof(struct sockaddr))) == -1) return ERR_SOCKACCEPT;
#endif

    int_to_char(sock_client, (char*)Oenviron->arguments[1]->value);

    sockets[sock_client].used = true;
    return ERR_SUCCESS;
}

unsigned int lib_net_cgetsockaddr(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;
    
    strcpy((char*)Oenviron->arguments[1]->value, inet_ntoa(sockets[sock_select].sAddr.sin_addr));
    
    return ERR_SUCCESS;
}

unsigned int lib_net_cgetsockport(OEnvironment *Oenviron) {
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;
    
    int_to_char((unsigned int)sockets[sock_select].sAddr.sin_port, (char*)Oenviron->arguments[1]->value);

    return ERR_SUCCESS;
}

unsigned int lib_net_csetsockssl(OEnvironment *Oenviron) {
#if !OPENSSL
    return ERR_NOCALL;
#endif
    unsigned int sock_select = *(unsigned int*)Oenviron->arguments[0]->value;
    if ((sock_select > N_SOCKS) || !(sockets[sock_select].used)) return ERR_INVALIDSOCK;
    sockets[sock_select].use_ssl = true;

    return ERR_SUCCESS;
}

OCall lib_net_calls[LIB_NET_LNCALLS] = {
    {.name = "socket", .description = "socket(domain, type, protocol, outitem)", .function=lib_net_csocket},
    {.name = "sconnect", .description = "sconnect(sockobj, domain, addr, port)", .function=lib_net_csconnect},
    {.name = "ssend", .description = "ssend(sockobj, buffer, bufferlength, flags)", .function=lib_net_cssend},
    {.name = "sclose", .description = "sclose(sockobj)", .function=lib_net_csclose},
    {.name = "srecv", .description = "srecv(sockobj, size, address, outsize)", .function=lib_net_csrecv},
    {.name = "gethostbyname", .description = "gethostbyname(name, outaddress)", .function=lib_net_cgethostbyname},
    {.name = "setsockopt", .description = "setsockopt(sockobj, level, option, option_value, option_length)", .function=lib_net_csetsockopt},
    {.name = "slisten", .description = "slisten(sockobj, backlog)", .function=lib_net_cslisten},
    {.name = "sbind", .description = "sbind(sockobj, domain, address, port)", .function=lib_net_csbind},
    {.name = "saccept", .description = "saccept(sockobj, outobj)", .function=lib_net_csaccept},
    {.name = "getsockaddr", .description = "getsockaddr(sockobj, outaddr)", .function=lib_net_cgetsockaddr},
    {.name = "getsockport", .description = "getsockport(sockobj, outport)", .function=lib_net_cgetsockport},
    {.name = "setsockssl", .description = "setsockssl(sockobj)", .function=lib_net_csetsockssl},
};
