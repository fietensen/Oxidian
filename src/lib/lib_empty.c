#include <Oxidian/lib/lib_empty.h>

unsigned int lib_empty_cempty(OEnvironment *Oenviron) {
    return ERR_NOLIB;
}

OCall lib_empty_calls[LIB_EMPTY_LNCALLS] = {
    {.name = "empty", .description = "doesn't exist :P", .function=lib_empty_cempty}
};
