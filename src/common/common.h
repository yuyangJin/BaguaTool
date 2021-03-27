#ifndef COMMON_H
#define COMMON_H

#include "dbg.h"

#define ERR_EXIT(...) \
    do {                    \
        dbg(__VA_ARGS__);   \
    } while (false)

#define UNREACHABLE() ERR_EXIT("control flow should never reach here")

#define UNIMPLEMENTED() ERR_EXIT("unimplemented here")

#endif
