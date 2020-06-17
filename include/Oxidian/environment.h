#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Oxidian/config.h>

// Du to compiler warnings (I hate compiler warnings ://) on windoof I'll use memcpy instead
//#define C2UI(c) (*(uint32_t*)c)
//#define C2US(c) (*(uint16_t*)c)

typedef struct {
    uint32_t size;
    void* value;
} OVariable;

typedef struct {
    FILE* fp;

    // Variables important for parsing the Bytecode
    char magic_number[3];
    uint8_t program_setting;
    char temp_addr[4];
    char temp_short[2];
    uint32_t data_section;
    uint32_t code_section;
    uint32_t program_pointer;
    uint32_t data_length;
    int32_t buf;
    uint32_t data_elements;
    uint32_t size;
    uint32_t instructions_length;
    //uint32_t *element_sizes;
    //char **data_items;

    // Runtime Variables
    uint16_t current_library;
    uint16_t call_number;
    OVariable *variables;
    OVariable *arguments[256];
    uint8_t argument_number;
    uint32_t data_number;
    uint32_t stack_size;
    int32_t temp_i[256];
    uint32_t return_addresses[256];
    int16_t temp_s[256];
    uint8_t function_depth;
    int8_t compare_result;
    bool debug;
    bool nullstack;
    bool dumpstack;
    bool iscase;
    uint8_t *stack;
    uint32_t stackpointerposition;
    uint16_t current_instruction;
    uint16_t instruction_before;
    uint32_t error_number;
} OEnvironment;

void setup_environment(OEnvironment*);
void clean_environment(OEnvironment*);
uint32_t C2UI(char*);
uint16_t C2US(char*);

#endif
