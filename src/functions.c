#include <Oxidian/functions.h>
#include <stdio.h>

OLibrary libraries[LIBS] = {
#ifdef IOLIB
    {.name = LIB_IO_LNAME, .author = LIB_IO_LAUTHOR, .description = LIB_IO_LDESCRIPTION, .ncalls = LIB_IO_LNCALLS, .calls = lib_io_calls},
#else
    {.name = LIB_EMPTY_LNAME, .author = LIB_EMPTY_LAUTHOR, .description = LIB_EMPTY_LDESCRIOTION, .ncalls = LIB_EMPTY_LNCALLS, .calls = lib_empty_calls},
#endif
#ifdef MEMLIB
    {.name = LIB_MEM_LNAME, .author = LIB_MEM_LAUTHOR, .description = LIB_MEM_LDESCRIPTION, .ncalls = LIB_MEM_LNCALLS, .calls = lib_mem_calls},
#else
    {.name = LIB_EMPTY_LNAME, .author = LIB_EMPTY_LAUTHOR, .description = LIB_EMPTY_LDESCRIOTION, .ncalls = LIB_EMPTY_LNCALLS, .calls = lib_empty_calls},
#endif
#ifdef NETLIB
    {.name = LIB_NET_LNAME, .author = LIB_NET_LAUTHOR, .description = LIB_NET_LDESCRIPTION, .ncalls = LIB_NET_LNCALLS, .calls = lib_net_calls},
#else
    {.name = LIB_EMPTY_LNAME, .author = LIB_EMPTY_LAUTHOR, .description = LIB_EMPTY_LDESCRIOTION, .ncalls = LIB_EMPTY_LNCALLS, .calls = lib_empty_calls},
#endif
};

bool handle_call(OEnvironment* Oenviron) {
    if (Oenviron->current_library < LIBS) {
        if (Oenviron->call_number < libraries[Oenviron->current_library].ncalls) {
            if (runtimeerror(libraries[Oenviron->current_library].calls[Oenviron->call_number].function(Oenviron), Oenviron)) {
                clean_environment(Oenviron);
                fclose(Oenviron->fp);
                return 1;
            }
            return 0;
        } else {
            if (runtimeerror(ERR_NOCALL, Oenviron)) {
                clean_environment(Oenviron);
                fclose(Oenviron->fp);
                return 1;
            }
        }
    } else {
        if (runtimeerror(ERR_NOLIB, Oenviron)) {
            clean_environment(Oenviron);
            fclose(Oenviron->fp);
            return 1;
        }
    }
    return 0;
}