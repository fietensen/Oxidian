#include <Oxidian/lib/lib_mem.h>

unsigned int lib_mem_csetstack(OEnvironment *Oenviron) {
    unsigned int stack_write_addr = *(unsigned int*)Oenviron->arguments[0]->value;
    unsigned int element_writelen = *(unsigned int*)Oenviron->arguments[1]->value;
    unsigned int items_write = *(unsigned int*)Oenviron->arguments[2]->value;
    unsigned int req_size = element_writelen*items_write;
    if (Oenviron->stack_size-Oenviron->stackpointerposition < req_size) {   
        return ERR_STACKSPACE;
    }

    for (unsigned int item=3;item<items_write+3;item++) { // loop over elements
        memcpy(Oenviron->stack+stack_write_addr, Oenviron->arguments[item]->value, element_writelen);
        stack_write_addr += element_writelen;
    }

    return ERR_SUCCESS;
}

unsigned int lib_mem_ccpystack(OEnvironment *Oenviron) {

    unsigned int length = *(unsigned int*)Oenviron->arguments[0]->value;
    unsigned int addr0 = *(unsigned int*)Oenviron->arguments[1]->value;
    unsigned int addr1 = *(unsigned int*)Oenviron->arguments[2]->value;

    if (Oenviron->stack_size-addr1 < length) {
        return ERR_STACKSPACE;
    } 

    for (unsigned int i=0;i<length;i++) {
        Oenviron->stack[addr1+i] = Oenviron->stack[addr0+i];
    }

    return ERR_SUCCESS;
}

unsigned int lib_mem_cswpstack(OEnvironment *Oenviron) {
    unsigned int length = *(unsigned int*)Oenviron->arguments[0]->value;
    unsigned int addr0 = *(unsigned int*)Oenviron->arguments[1]->value;
    unsigned int addr1 = *(unsigned int*)Oenviron->arguments[2]->value;

    if ((Oenviron->stack_size-addr0 < length) || (Oenviron->stack_size-addr1 < length)) {
        return ERR_STACKSPACE;
    } 

    for (unsigned int i=0;i<length;i++) {
        unsigned char buffer = Oenviron->stack[addr1+i];
        Oenviron->stack[addr1+i] = Oenviron->stack[addr0+i];
        Oenviron->stack[addr0+i] = buffer;
    }

    return ERR_SUCCESS;
}

unsigned int lib_mem_crevstack(OEnvironment *Oenviron) {

    unsigned int stack_addr = *(unsigned int*)Oenviron->arguments[0]->value;
    unsigned int length = *(unsigned int*)Oenviron->arguments[1]->value;

    unsigned int i=0;
    while (i<length-1) {
        unsigned char buffer = Oenviron->stack[i];
        Oenviron->stack[stack_addr+i] = Oenviron->stack[stack_addr+length-1];
        Oenviron->stack[stack_addr+length-1] = buffer;
        length--;
        i++;
    }

    return ERR_SUCCESS;
}

OCall lib_mem_calls[LIB_MEM_LNCALLS] = {
    {.name = "setstack", .description = "writes (argument 1) bytes from each of the (argument 2) arguments to the stack beginning at address (argument 0)", .function=lib_mem_csetstack},
    {.name = "cpystack", .description = "copies (argument 0) bytes from stack at address (argument 1) to stack at address (argument 2)", .function=lib_mem_ccpystack},
    {.name = "swpstack", .description = "swaps (argument 0) bytes from stack at address (argument 1) with bytes at stack at address (argument 2)", .function=lib_mem_cswpstack},
    {.name = "reversestack", .description = "reverses (argument 1) bytes at stack address (argument 0)", .function=lib_mem_crevstack}
};
