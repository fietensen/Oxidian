#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H
#include <stdio.h>
#include <Oxidian/environment.h>
#include <stdint.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

unsigned int readint(FILE*);
void int_to_char(unsigned int,char*);
FILE* getfp(char[sizeof(FILE*)]);

#endif