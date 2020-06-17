#include <Oxidian/environment.h>
#include <Oxidian/errors.h>

uint32_t C2UI(char *buffer) {
    uint32_t value;
    memcpy(&value, buffer, sizeof(uint32_t));
    return value;
}

uint16_t C2US(char *buffer) {
    uint16_t value;
    memcpy(&value, buffer, sizeof(uint16_t));
    return value;
}

void setup_environment(OEnvironment* Oenviron) {
    // Set the environment to it's initial values
    
    Oenviron->data_section = 0;
    Oenviron->code_section = 0;
    Oenviron->program_pointer = 0;
    Oenviron->data_length = 0;
    Oenviron->buf = 0;
    Oenviron->data_elements = 0;
    Oenviron->size = 0;
    Oenviron->instructions_length = 0;

    Oenviron->current_library = 0xdeb9; // <- means to say debug in case you forget to set the library
    Oenviron->call_number = 0xdeb9; // same as above
    Oenviron->function_depth = 0;
    Oenviron->debug = false;
    Oenviron->nullstack = false;
    Oenviron->dumpstack = false;
    Oenviron->iscase = false;
    Oenviron->stackpointerposition = 0;

    Oenviron->current_instruction = 0;
    Oenviron->instruction_before = -1;
    Oenviron->error_number = ERR_SUCCESS; // debug

    memset(&Oenviron->arguments, 0, sizeof(char*)*256);
    return;
}

void clean_environment(OEnvironment* Oenviron) {
    if (Oenviron->debug) printf("[DEBUG] Cleaning up...\n");
    for (unsigned int i=0;i<Oenviron->data_elements;i++) {
        free(Oenviron->variables[i].value);
    }

    free(Oenviron->stack);
    //free(Oenviron->data_items);
    free(Oenviron->variables);
    return;
}
