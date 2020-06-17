#include <stdio.h>
#include <Oxidian/VM.h>

int main(int argc, char **argv, char **envp) {
    
    // init_oxy does the stuff, main just fetches the filename
    // from user input or argument

    char sourcefile[256];
    int errors;

    if (argc == 2) {
        errors = init_oxy(argv[1], argc, argv, envp);
    } else {
        printf(
            "Oxidian Virtual Machine [Version 1.2.0] - Everything else is irrelevant.\n\n"
        );
        printf("[OXY]: Enter input file: ");
        fgets(sourcefile, 256, stdin);
        printf("\n");
        errors = init_oxy(sourcefile, argc, argv, envp);
    }
    
    return errors;
}