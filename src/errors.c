#include <stdio.h>
#include <Oxidian/errors.h>

void runstats(OEnvironment* Oenviron) {
    printf("Stats:\n");
    printf("\tCurrent Offset: 0x%x (%u)\n", Oenviron->program_pointer, Oenviron->program_pointer);
    printf("\tInstruction before: 0x%hx (%hu)\n", Oenviron->instruction_before, Oenviron->instruction_before);
    printf("\tInstruction that caused the error: 0x%hx (%hu)\n", Oenviron->current_instruction, Oenviron->current_instruction);
    printf("\tLibrary selected: 0x%hx (%uh)\n", Oenviron->current_library, Oenviron->current_library);
    printf("\tFunction selected: 0x%hx (%uh)\n", Oenviron->call_number, Oenviron->call_number);
}

bool runtimeerror(unsigned int error_n, OEnvironment* Oenviron) {

    switch (error_n) {
        case ERR_NOINST:
            printf("[RUNTIME ERROR Errno. %u] Unknown instruction found.\n", error_n);
            runstats(Oenviron);
            return true;
        case ERR_NOLIB:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Function from invalid library called.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_NOLIB;
            return false;
        case ERR_NOCALL:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Trying to call nonexisting function.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_NOCALL;
            return false;
        case ERR_SUCCESS:
            Oenviron->error_number = ERR_SUCCESS;
            return false;
        case ERR_STACKSPACE:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Stack space too small to execute operation.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_STACKSPACE;
            return false;
        case ERR_INVALIDFMODE:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Invalid file opening mode.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_INVALIDFMODE;
            return false;
        case ERR_NOFILE:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Trying to open nonexisting file.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_NOFILE;
            return false;
        case ERR_FOPENERR:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Failed opening the file.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_FOPENERR;
            return false;
        case ERR_SEEK:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Function seek failed.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_SEEK;
            return false;
        case ERR_FCLOSE:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Closing file resulted in an error.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_FCLOSE;
            return false;
        case ERR_NOSOCK:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Failed to create new socket. (no free sockets left).\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_NOSOCK;
            return false;
        case ERR_SOCKET:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Failed to create new socket. (creation function failed).\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_NOSOCK;
            return false;
        case ERR_INVALIDSOCK:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Socket call failed. Socketobject is invalid.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_INVALIDSOCK;
            return false;
        case ERR_CONNECT:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Failed to connect to server.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_CONNECT;
            return false;
        case ERR_SOCKSEND:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Failed to send data.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_SOCKSEND;
            return false;
        case ERR_SOCKCLOSE:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] An error occoured while trying to close a socket.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_SOCKCLOSE;
            return false;
        case ERR_RECV:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] An error occoured while trying to receive data via socket.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_RECV;
            return false;
        case ERR_GETHOSTBYNAME:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Unable to receive hostent for hostname. (gethostbyaddr)\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_GETHOSTBYNAME;
            return false;
        case ERR_SETSOCKOPT:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Setsockopt failed.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_SETSOCKOPT;
            return false;
        case ERR_LISTEN:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Listen returned an error.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_LISTEN;
            return false;
        case ERR_BIND:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Failed to bind to socket.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_BIND;
            return false;
        case ERR_SOCKACCEPT:
            if (Oenviron->debug) {
                printf("[RUNTIME ERROR Errno. %u] Accept failed.\n", error_n);
                runstats(Oenviron);
            }

            Oenviron->error_number = ERR_SOCKACCEPT;
            return false;
        default:
            printf("[RUNTIME ERROR Errno. %u] An unknown error occoured.\n", error_n);
            runstats(Oenviron);
            return true;
    }
}