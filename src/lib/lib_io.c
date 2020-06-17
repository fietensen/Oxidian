#include <Oxidian/lib/lib_io.h>

unsigned int lib_io_cprint(OEnvironment *Oenviron) {
    printf("%s", (char*)Oenviron->arguments[0]->value);
    return ERR_SUCCESS;
}

unsigned int lib_io_cread(OEnvironment *Oenviron) {
    if (Oenviron->stack_size-Oenviron->stackpointerposition < *(unsigned int*)Oenviron->arguments[0]->value) {   
        return ERR_STACKSPACE;
    }
    memset(Oenviron->stack + Oenviron->stackpointerposition, 0, *(unsigned int*)Oenviron->arguments[0]->value);
    for (unsigned int i=0;i<*(unsigned int*)Oenviron->arguments[0]->value;i++) {
        if ((Oenviron->stack[Oenviron->stackpointerposition] = fgetc(stdin)) == '\n') {
            Oenviron->stack[Oenviron->stackpointerposition] = '\x00';
            int_to_char(i, Oenviron->arguments[0]->value);
            break;
        } else {
            Oenviron->stackpointerposition++;
        }
    }
    return ERR_SUCCESS;
}

unsigned int lib_io_cprintstack(OEnvironment *Oenviron) {
    Oenviron->stackpointerposition -= *(unsigned int*)Oenviron->arguments[0]->value;
    for (unsigned int i=0;i<*(unsigned int*)Oenviron->arguments[0]->value;i++) {
        printf("%c", Oenviron->stack[Oenviron->stackpointerposition]);
        Oenviron->stackpointerposition++;
    }
    return ERR_SUCCESS;
}

unsigned int lib_io_cfopen(OEnvironment *Oenviron) {
    Oenviron->temp_i[0] = *(int*)Oenviron->arguments[1]->value; // mode
    FILE* handle_file;
    
    switch(Oenviron->temp_i[0] & 0b11) {
        case O_READ:
            handle_file = fopen(Oenviron->arguments[0]->value, "r");
            if ((handle_file == NULL) && !(Oenviron->temp_i[0] | O_CREAT)) {
                return ERR_NOFILE;
            } else if (handle_file == NULL) {
                handle_file = fopen(Oenviron->arguments[0]->value, "w");
                fclose(handle_file);
            }
            break;
        case O_WRITE:
            handle_file = fopen(Oenviron->arguments[0]->value, "w");
            if (handle_file == NULL) {
                return ERR_FOPENERR;
            }
            break;
        case O_APP:
            handle_file = fopen(Oenviron->arguments[0]->value, "a");
            if (handle_file == NULL) {
                return ERR_FOPENERR;
            }
            break;
        case O_READWRITE:
            handle_file = fopen(Oenviron->arguments[0]->value, "w+");
            if (handle_file == NULL) {
                return ERR_FOPENERR;
            }
            break;
        default:
            return ERR_INVALIDFMODE;
    }
    
    memcpy(Oenviron->arguments[2]->value, &handle_file, sizeof(FILE*));

    return ERR_SUCCESS;
}

unsigned int lib_io_cfread(OEnvironment *Oenviron) {

    unsigned int bytes_read;

    bytes_read = fread(&Oenviron->stack[*(unsigned int*)Oenviron->arguments[2]->value], *(unsigned int*)Oenviron->arguments[1]->value, 1, getfp(Oenviron->arguments[0]->value));
    int_to_char(bytes_read, Oenviron->arguments[1]->value);

    return ERR_SUCCESS;
}

unsigned int lib_io_cfwrite(OEnvironment *Oenviron) {
    unsigned int bytes_written;
    
    bytes_written = fwrite(&(Oenviron->stack[*(unsigned int*)Oenviron->arguments[2]->value]), *(unsigned int*)Oenviron->arguments[1]->value, 1, getfp(Oenviron->arguments[0]->value));
    int_to_char(bytes_written, Oenviron->arguments[1]->value);
    return ERR_SUCCESS;
}

unsigned int lib_io_cftell(OEnvironment *Oenviron) {
    int_to_char(ftell(getfp(Oenviron->arguments[0]->value)), Oenviron->arguments[1]->value);

    return ERR_SUCCESS;
}

unsigned int lib_io_cfseek(OEnvironment *Oenviron) {
    unsigned int return_value;
    return_value = fseek(getfp(Oenviron->arguments[0]->value), *(unsigned int*)Oenviron->arguments[1]->value, *(unsigned int*)Oenviron->arguments[2]->value);

    if (return_value) {
        return ERR_SEEK;
    }

    return ERR_SUCCESS;
}

unsigned int lib_io_cfclose(OEnvironment *Oenviron) {

    if (fclose(getfp(Oenviron->arguments[0]->value))) {
        return ERR_FCLOSE;
    }

    return ERR_SUCCESS;
}

unsigned int lib_io_csystem(OEnvironment *Oenviron) {
    
    system(Oenviron->arguments[0]->value);

    return ERR_SUCCESS;
}

OCall lib_io_calls[LIB_IO_LNCALLS] = {
    {.name = "print", .description = "print string in argument 0", .function=lib_io_cprint},
    {.name = "read", .description = "read (argument 0) bytes from stdin and puts number of bytes read into argument 0", .function=lib_io_cread},
    {.name = "printstack", .description = "print string on stack, length = argument 0", .function=lib_io_cprintstack},
    {.name = "fopen", .description = "open file in argument 0 in mode specified in argument 1, saving the file handle in argument 2", .function=lib_io_cfopen},
    {.name = "fread", .description = "read (argument 1) bytes from file at argument 0 and write bytes to the stack at address (argument 2). bytes read are put into argument 1", .function=lib_io_cfread},
    {.name = "fwrite", .description = "write (argument 1) bytes from stack at address (argument 2) to file at (argument 0). puts bytes written into argument 1", .function=lib_io_cfwrite},
    {.name = "ftell", .description = "puts the position of file at (argument 0) into argument 1", .function=lib_io_cftell},
    {.name = "fseek", .description = "sets the position of file at (argument 0) to offset (argument 1) beginning from argument 2.", .function=lib_io_cfseek},
    {.name = "fclose", .description = "close file at (argument 0)", .function=lib_io_cfclose},
    {.name = "system", .description = "executes bash/batch code in argument 0", .function=lib_io_csystem},
};
