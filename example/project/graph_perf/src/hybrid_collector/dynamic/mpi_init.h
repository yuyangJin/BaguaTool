#ifndef MPI_INIT_H_
#define MPI_INIT_H_

#include <papi.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef __cplusplus
extern "C" {
#endif

extern int mpi_rank;

#ifdef __cplusplus
}
#endif

#endif  // MPI_INIT_H_
