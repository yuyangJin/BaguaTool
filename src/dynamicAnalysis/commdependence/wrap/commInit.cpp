
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */

#ifdef MPICH_HAS_C2F
_EXTERN_C_ void *MPIR_ToPointer(int);
#endif // MPICH_HAS_C2F

#ifdef PIC
/* For shared libraries, declare these weak and figure out which one was linked
   based on which init wrapper was called.  See mpi_init wrappers.  */
#pragma weak pmpi_init
#pragma weak PMPI_INIT
#pragma weak pmpi_init_
#pragma weak pmpi_init__
#endif /* PIC */

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);

// -*- c++ -*-
//
// commData
// Yuyang Jin
//
// This file is to generate code to collect runtime communication data.
//
// To build:
//    ./wrap.py -f commData.w > commData.cpp
//    mpicc -c commData.cpp
//    #ar cr libcommData.a commData.o
//    #ranlib libcommData.a
//
// Link your application with libcommData.a, or build it as a shared lib
// and LD_PRELOAD it to try out this tool.
//
// In ScalAna, we link commData.o with other .obj files to get runtime data.
//
#include <mpi.h>
#include "IRStruct.h"
#include <iostream>

using namespace std;
//int mpiRank = 0;

// MPI_Init does all the communicator setup
//
static int fortran_init = 0;
/* ================== C Wrappers for MPI_Init ================== */
_EXTERN_C_ int PMPI_Init(int *argc, char ***argv);
_EXTERN_C_ int MPI_Init(int *argc, char ***argv) { 
    int _wrap_py_return_val = 0;
{
    // First call PMPI_Init()
        if (fortran_init) {
#ifdef PIC
        if (!PMPI_INIT && !pmpi_init && !pmpi_init_ && !pmpi_init__) {
            fprintf(stderr, "ERROR: Couldn't find fortran pmpi_init function.  Link against static library instead.\n");
            exit(1);
        }        switch (fortran_init) {
        case 1: PMPI_INIT(&_wrap_py_return_val);   break;
        case 2: pmpi_init(&_wrap_py_return_val);   break;
        case 3: pmpi_init_(&_wrap_py_return_val);  break;
        case 4: pmpi_init__(&_wrap_py_return_val); break;
        default:
            fprintf(stderr, "NO SUITABLE FORTRAN MPI_INIT BINDING\n");
            break;
        }
#else /* !PIC */
        pmpi_init_(&_wrap_py_return_val);
#endif /* !PIC */
    } else {
        _wrap_py_return_val = PMPI_Init(argc, argv);
    }


    PMPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    //cout << mpiRank << "*****\n";
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Init =============== */
static void MPI_Init_fortran_wrapper(MPI_Fint *ierr) { 
    int argc = 0;
    char ** argv = NULL;
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Init(&argc, &argv);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INIT(MPI_Fint *ierr) { 
    fortran_init = 1;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init(MPI_Fint *ierr) { 
    fortran_init = 2;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init_(MPI_Fint *ierr) { 
    fortran_init = 3;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init__(MPI_Fint *ierr) { 
    fortran_init = 4;
    MPI_Init_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Init ================= */


/* ================== C Wrappers for MPI_Init_thread ================== */
_EXTERN_C_ int PMPI_Init_thread(int *argc, char ***argv, int required, int *provided);
_EXTERN_C_ int MPI_Init_thread(int *argc, char ***argv, int required, int *provided) { 
    int _wrap_py_return_val = 0;
{
    // First call PMPI_Init()
    _wrap_py_return_val = PMPI_Init_thread(argc, argv, required, provided);

    PMPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    //cout << mpiRank << "*****\n";
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Init_thread =============== */
static void MPI_Init_thread_fortran_wrapper(MPI_Fint *argc, MPI_Fint ***argv, MPI_Fint *required, MPI_Fint *provided, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Init_thread((int*)argc, (char***)argv, *required, (int*)provided);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INIT_THREAD(MPI_Fint *argc, MPI_Fint ***argv, MPI_Fint *required, MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Init_thread_fortran_wrapper(argc, argv, required, provided, ierr);
}

_EXTERN_C_ void mpi_init_thread(MPI_Fint *argc, MPI_Fint ***argv, MPI_Fint *required, MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Init_thread_fortran_wrapper(argc, argv, required, provided, ierr);
}

_EXTERN_C_ void mpi_init_thread_(MPI_Fint *argc, MPI_Fint ***argv, MPI_Fint *required, MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Init_thread_fortran_wrapper(argc, argv, required, provided, ierr);
}

_EXTERN_C_ void mpi_init_thread__(MPI_Fint *argc, MPI_Fint ***argv, MPI_Fint *required, MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Init_thread_fortran_wrapper(argc, argv, required, provided, ierr);
}

/* ================= End Wrappers for MPI_Init_thread ================= */



