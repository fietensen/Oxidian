#include <Oxidian/helper_functions.h>

unsigned int readint(FILE* fp) {
    char addr[4];
    uint32_t value;
    fread(addr, 4, 1, fp);
    memcpy(&value, addr, sizeof(uint32_t));
    return value;
}

void int_to_char(unsigned int value, char* copyto) {
    copyto[3] = value>>24&0xFF;
    copyto[2] = value>>16&0xFF;
    copyto[1] = value>>8&0xFF;
    copyto[0] = value&0xFF;
}

FILE* getfp(char* container) {
    FILE* handle;
    memcpy(&handle, container, sizeof(FILE*));
    return handle;
}
