#ifndef IRSTRUCT_H
#define IRSTRUCT_H

#include <papi.h>

#define ENTRY_FUNC entryPoint
#define EXIT_FUNC exitPoint
#define LATCH_FUNC latchPoint
#define COMBINE_FUNC combinePoint

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ENTRY_FUNC_NAME TOSTRING(ENTRY_FUNC)
#define EXIT_FUNC_NAME TOSTRING(EXIT_FUNC)
#define LATCH_FUNC_NAME TOSTRING(LATCH_FUNC)
#define COMBINE_FUNC_NAME TOSTRING(COMBINE_FUNC)

// these macros can be used for colorful output
#define TPRT_NOCOLOR "\033[0m"
#define TPRT_RED "\033[1;31m"
#define TPRT_GREEN "\033[1;32m"
#define TPRT_YELLOW "\033[1;33m"
#define TPRT_BLUE "\033[1;34m"
#define TPRT_MAGENTA "\033[1;35m"
#define TPRT_CYAN "\033[1;36m"
#define TPRT_REVERSE "\033[7m"

#define LOG_INFO(fmt, ...) fprintf(stderr,TPRT_GREEN fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_ERROR(fmt, ...) fprintf(stderr,TPRT_RED fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_WARN(fmt, ...) fprintf(stderr,TPRT_MAGENTA fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_LINE fprintf(stderr,TPRT_BLUE "line=%d\n" TPRT_NOCOLOR, __LINE__);
//
#define TRY(func, flag) \
{ \
        int retval = func;\
        if (retval != flag) LOG_ERROR("%s, ErrCode: %s\n", #func, PAPI_strerror(retval));\
}

#ifdef __cplusplus
extern "C" {
#endif

/*struct MPIINFO{
        unsigned int id;
        int commSetSize;
        int *commSet;
        unsigned int dataSize;
        unsigned int count;
};*/

enum NodeType {
    COMPUTING = -9,
    COMBINE = -8,
    CALL_INDIRECT = -7,
    CALL_REC = -6,
    CALL = -5,
    FUNCTION = -4,
    COMPOUND = -3,
    BRANCH = -2,
    LOOP = -1,
};

//void ENTRY_FUNC(int childID, int id, int type);
//void ENTRY_FUNC(int id, int type);

//void EXIT_FUNC(int childID, int id, int type);
//void EXIT_FUNC(int id, int type);

//void LATCH_FUNC(int childID, int id, int type);

//void COMBINE_FUNC(int childID, int id, int type);


extern int mpiRank;

//extern int curCommId;
//extern int EventSet;

//#define MPI_INFO_MAX 1000000

//extern struct MPIINFO mpiInfo[MPI_INFO_MAX];
//extern unsigned int mpiInfoPointer;

#ifdef __cplusplus
}
#endif

#endif // IRSTRUCT_H
