
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
// world48
// Todd Gamblin, tgamblinWllnl.gov
//
// This file is an example of how to use wrap.py to fool an application
// into thinking it is running on a different communicator from
// MPI_Comm_world.
//
// This was originally intended to allow applications on Blue Gene/Q to
// run with 48 MPI processes per node, rather than just the power of 2
// that IBM provides settings for.  The MPI_Init wrapper here will
// split MPI_Comm_world into 2 groups: one for the first 48 out of every
// 64 ranks and one for the last 16.  The last 16 ranks of every 64 just
// call MPI_Finalize and exit normally inside of MPI_Init.  The rest of
// the ranks continue to execute the rest of the application, thinking
// that the world is only 1/4 as big as the real MPI_COMM_WORLD.
//
// To build:
//    wrap.py world48.w > world48.C
//    mpicc -c world48.C
//    ar cr libworld48.a world48.o
//    ranlib libworld48.a
//
// Link your application with libworld48.a, or build it as a shared lib
// and LD_PRELOAD it to try out this tool.
//
#include <mpi.h>

// This is a communicator that will contain the first 48 out of
// every 64 ranks in the application.
static MPI_Comm world48;

// This function modifies its parameter by swapping it with world48
// if it is MPI_COMM_WORLD.
inline void swap_world(MPI_Comm& world) {
   if (world == MPI_COMM_WORLD) {
      world = world48;
   }
}

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


   int rank;
   PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

   // now keep only the first 48 ranks of each 64.
   int keep = (rank % 64 < 48) ? 1: 0;
   PMPI_Comm_split(MPI_COMM_WORLD,  keep, rank, &world48);

   // throw away the remaining ranks.
   if (!keep) {
      PMPI_Finalize();
      exit(0);
   }
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




// This generates interceptors that will catch every MPI routine
// *except* MPI_Init.  The interceptors just make sure that if
// they are called with an argument of type MPI_Comm that has a
// value of MPI_COMM_WORLD, they switch it with world48.
/* ================== C Wrappers for MPI_Abort ================== */
_EXTERN_C_ int PMPI_Abort(MPI_Comm comm, int errorcode);
_EXTERN_C_ int MPI_Abort(MPI_Comm comm, int errorcode) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Abort(comm, errorcode);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Abort =============== */
static void MPI_Abort_fortran_wrapper(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Abort((MPI_Comm)(*comm), *errorcode);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Abort(MPI_Comm_f2c(*comm), *errorcode);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ABORT(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Abort_fortran_wrapper(comm, errorcode, ierr);
}

_EXTERN_C_ void mpi_abort(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Abort_fortran_wrapper(comm, errorcode, ierr);
}

_EXTERN_C_ void mpi_abort_(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Abort_fortran_wrapper(comm, errorcode, ierr);
}

_EXTERN_C_ void mpi_abort__(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Abort_fortran_wrapper(comm, errorcode, ierr);
}

/* ================= End Wrappers for MPI_Abort ================= */


/* ================== C Wrappers for MPI_Accumulate ================== */
_EXTERN_C_ int PMPI_Accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win);
_EXTERN_C_ int MPI_Accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Accumulate(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Accumulate =============== */
static void MPI_Accumulate_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Accumulate((const void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Op)(*op), (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Accumulate((const void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Op_f2c(*op), MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ACCUMULATE(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

_EXTERN_C_ void mpi_accumulate(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

_EXTERN_C_ void mpi_accumulate_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

_EXTERN_C_ void mpi_accumulate__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

/* ================= End Wrappers for MPI_Accumulate ================= */


/* ================== C Wrappers for MPI_Add_error_class ================== */
_EXTERN_C_ int PMPI_Add_error_class(int *errorclass);
_EXTERN_C_ int MPI_Add_error_class(int *errorclass) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Add_error_class(errorclass);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Add_error_class =============== */
static void MPI_Add_error_class_fortran_wrapper(MPI_Fint *errorclass, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Add_error_class((int*)errorclass);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ADD_ERROR_CLASS(MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Add_error_class_fortran_wrapper(errorclass, ierr);
}

_EXTERN_C_ void mpi_add_error_class(MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Add_error_class_fortran_wrapper(errorclass, ierr);
}

_EXTERN_C_ void mpi_add_error_class_(MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Add_error_class_fortran_wrapper(errorclass, ierr);
}

_EXTERN_C_ void mpi_add_error_class__(MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Add_error_class_fortran_wrapper(errorclass, ierr);
}

/* ================= End Wrappers for MPI_Add_error_class ================= */


/* ================== C Wrappers for MPI_Add_error_code ================== */
_EXTERN_C_ int PMPI_Add_error_code(int errorclass, int *errorcode);
_EXTERN_C_ int MPI_Add_error_code(int errorclass, int *errorcode) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Add_error_code(errorclass, errorcode);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Add_error_code =============== */
static void MPI_Add_error_code_fortran_wrapper(MPI_Fint *errorclass, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Add_error_code(*errorclass, (int*)errorcode);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ADD_ERROR_CODE(MPI_Fint *errorclass, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Add_error_code_fortran_wrapper(errorclass, errorcode, ierr);
}

_EXTERN_C_ void mpi_add_error_code(MPI_Fint *errorclass, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Add_error_code_fortran_wrapper(errorclass, errorcode, ierr);
}

_EXTERN_C_ void mpi_add_error_code_(MPI_Fint *errorclass, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Add_error_code_fortran_wrapper(errorclass, errorcode, ierr);
}

_EXTERN_C_ void mpi_add_error_code__(MPI_Fint *errorclass, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Add_error_code_fortran_wrapper(errorclass, errorcode, ierr);
}

/* ================= End Wrappers for MPI_Add_error_code ================= */


/* ================== C Wrappers for MPI_Add_error_string ================== */
_EXTERN_C_ int PMPI_Add_error_string(int errorcode, const char *string);
_EXTERN_C_ int MPI_Add_error_string(int errorcode, const char *string) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Add_error_string(errorcode, string);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Add_error_string =============== */
static void MPI_Add_error_string_fortran_wrapper(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Add_error_string(*errorcode, (const char*)string);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ADD_ERROR_STRING(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *ierr) { 
    MPI_Add_error_string_fortran_wrapper(errorcode, string, ierr);
}

_EXTERN_C_ void mpi_add_error_string(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *ierr) { 
    MPI_Add_error_string_fortran_wrapper(errorcode, string, ierr);
}

_EXTERN_C_ void mpi_add_error_string_(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *ierr) { 
    MPI_Add_error_string_fortran_wrapper(errorcode, string, ierr);
}

_EXTERN_C_ void mpi_add_error_string__(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *ierr) { 
    MPI_Add_error_string_fortran_wrapper(errorcode, string, ierr);
}

/* ================= End Wrappers for MPI_Add_error_string ================= */


/* ================== C Wrappers for MPI_Address ================== */
_EXTERN_C_ int PMPI_Address(void *location, MPI_Aint *address);
_EXTERN_C_ int MPI_Address(void *location, MPI_Aint *address) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Address(location, address);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Address =============== */
static void MPI_Address_fortran_wrapper(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Address((void*)location, (MPI_Aint*)address);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ADDRESS(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Address_fortran_wrapper(location, address, ierr);
}

_EXTERN_C_ void mpi_address(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Address_fortran_wrapper(location, address, ierr);
}

_EXTERN_C_ void mpi_address_(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Address_fortran_wrapper(location, address, ierr);
}

_EXTERN_C_ void mpi_address__(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Address_fortran_wrapper(location, address, ierr);
}

/* ================= End Wrappers for MPI_Address ================= */


/* ================== C Wrappers for MPI_Allgather ================== */
_EXTERN_C_ int PMPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Allgather =============== */
static void MPI_Allgather_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Allgather((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Allgather((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLGATHER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_allgather(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_allgather_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_allgather__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Allgather ================= */


/* ================== C Wrappers for MPI_Allgatherv ================== */
_EXTERN_C_ int PMPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Allgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Allgatherv =============== */
static void MPI_Allgatherv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Allgatherv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Allgatherv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLGATHERV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_allgatherv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_allgatherv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_allgatherv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Allgatherv ================= */


/* ================== C Wrappers for MPI_Alloc_mem ================== */
_EXTERN_C_ int PMPI_Alloc_mem(MPI_Aint size, MPI_Info info, void *baseptr);
_EXTERN_C_ int MPI_Alloc_mem(MPI_Aint size, MPI_Info info, void *baseptr) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Alloc_mem(size, info, baseptr);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Alloc_mem =============== */
static void MPI_Alloc_mem_fortran_wrapper(MPI_Aint *size, MPI_Fint *info, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alloc_mem(*size, (MPI_Info)(*info), (void*)baseptr);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Alloc_mem(*size, MPI_Info_f2c(*info), (void*)baseptr);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLOC_MEM(MPI_Aint *size, MPI_Fint *info, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Alloc_mem_fortran_wrapper(size, info, baseptr, ierr);
}

_EXTERN_C_ void mpi_alloc_mem(MPI_Aint *size, MPI_Fint *info, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Alloc_mem_fortran_wrapper(size, info, baseptr, ierr);
}

_EXTERN_C_ void mpi_alloc_mem_(MPI_Aint *size, MPI_Fint *info, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Alloc_mem_fortran_wrapper(size, info, baseptr, ierr);
}

_EXTERN_C_ void mpi_alloc_mem__(MPI_Aint *size, MPI_Fint *info, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Alloc_mem_fortran_wrapper(size, info, baseptr, ierr);
}

/* ================= End Wrappers for MPI_Alloc_mem ================= */


/* ================== C Wrappers for MPI_Allreduce ================== */
_EXTERN_C_ int PMPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
_EXTERN_C_ int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Allreduce =============== */
static void MPI_Allreduce_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Allreduce((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Allreduce((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLREDUCE(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_allreduce(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_allreduce_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_allreduce__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

/* ================= End Wrappers for MPI_Allreduce ================= */


/* ================== C Wrappers for MPI_Alltoall ================== */
_EXTERN_C_ int PMPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Alltoall =============== */
static void MPI_Alltoall_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alltoall((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Alltoall((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLTOALL(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoall(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoall_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoall__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Alltoall ================= */


/* ================== C Wrappers for MPI_Alltoallv ================== */
_EXTERN_C_ int PMPI_Alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Alltoallv(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Alltoallv =============== */
static void MPI_Alltoallv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Alltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLTOALLV(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallv(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallv_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallv__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Alltoallv ================= */


/* ================== C Wrappers for MPI_Alltoallw ================== */
_EXTERN_C_ int PMPI_Alltoallw(const void *sendbuf, const int sendcounts[], const int sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const int rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm);
_EXTERN_C_ int MPI_Alltoallw(const void *sendbuf, const int sendcounts[], const int sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const int rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Alltoallw(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Alltoallw =============== */
static void MPI_Alltoallw_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alltoallw((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (const MPI_Datatype*)recvtypes, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Alltoallw((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (const MPI_Datatype*)recvtypes, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLTOALLW(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallw(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallw_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallw__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

/* ================= End Wrappers for MPI_Alltoallw ================= */


/* ================== C Wrappers for MPI_Attr_delete ================== */
_EXTERN_C_ int PMPI_Attr_delete(MPI_Comm comm, int keyval);
_EXTERN_C_ int MPI_Attr_delete(MPI_Comm comm, int keyval) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Attr_delete(comm, keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Attr_delete =============== */
static void MPI_Attr_delete_fortran_wrapper(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Attr_delete((MPI_Comm)(*comm), *keyval);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Attr_delete(MPI_Comm_f2c(*comm), *keyval);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ATTR_DELETE(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Attr_delete_fortran_wrapper(comm, keyval, ierr);
}

_EXTERN_C_ void mpi_attr_delete(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Attr_delete_fortran_wrapper(comm, keyval, ierr);
}

_EXTERN_C_ void mpi_attr_delete_(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Attr_delete_fortran_wrapper(comm, keyval, ierr);
}

_EXTERN_C_ void mpi_attr_delete__(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Attr_delete_fortran_wrapper(comm, keyval, ierr);
}

/* ================= End Wrappers for MPI_Attr_delete ================= */


/* ================== C Wrappers for MPI_Attr_get ================== */
_EXTERN_C_ int PMPI_Attr_get(MPI_Comm comm, int keyval, void *attribute_val, int *flag);
_EXTERN_C_ int MPI_Attr_get(MPI_Comm comm, int keyval, void *attribute_val, int *flag) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Attr_get(comm, keyval, attribute_val, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Attr_get =============== */
static void MPI_Attr_get_fortran_wrapper(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Attr_get((MPI_Comm)(*comm), *keyval, (void*)attribute_val, (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Attr_get(MPI_Comm_f2c(*comm), *keyval, (void*)attribute_val, (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ATTR_GET(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Attr_get_fortran_wrapper(comm, keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_attr_get(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Attr_get_fortran_wrapper(comm, keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_attr_get_(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Attr_get_fortran_wrapper(comm, keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_attr_get__(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Attr_get_fortran_wrapper(comm, keyval, attribute_val, flag, ierr);
}

/* ================= End Wrappers for MPI_Attr_get ================= */


/* ================== C Wrappers for MPI_Attr_put ================== */
_EXTERN_C_ int PMPI_Attr_put(MPI_Comm comm, int keyval, void *attribute_val);
_EXTERN_C_ int MPI_Attr_put(MPI_Comm comm, int keyval, void *attribute_val) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Attr_put(comm, keyval, attribute_val);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Attr_put =============== */
static void MPI_Attr_put_fortran_wrapper(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Attr_put((MPI_Comm)(*comm), *keyval, (void*)attribute_val);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Attr_put(MPI_Comm_f2c(*comm), *keyval, (void*)attribute_val);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ATTR_PUT(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Attr_put_fortran_wrapper(comm, keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_attr_put(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Attr_put_fortran_wrapper(comm, keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_attr_put_(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Attr_put_fortran_wrapper(comm, keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_attr_put__(MPI_Fint *comm, MPI_Fint *keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Attr_put_fortran_wrapper(comm, keyval, attribute_val, ierr);
}

/* ================= End Wrappers for MPI_Attr_put ================= */


/* ================== C Wrappers for MPI_Barrier ================== */
_EXTERN_C_ int PMPI_Barrier(MPI_Comm comm);
_EXTERN_C_ int MPI_Barrier(MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Barrier(comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Barrier =============== */
static void MPI_Barrier_fortran_wrapper(MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Barrier((MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Barrier(MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_BARRIER(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_barrier(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_barrier_(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_barrier__(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(comm, ierr);
}

/* ================= End Wrappers for MPI_Barrier ================= */


/* ================== C Wrappers for MPI_Bcast ================== */
_EXTERN_C_ int PMPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
_EXTERN_C_ int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Bcast(buffer, count, datatype, root, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Bcast =============== */
static void MPI_Bcast_fortran_wrapper(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Bcast((void*)buffer, *count, (MPI_Datatype)(*datatype), *root, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Bcast((void*)buffer, *count, MPI_Type_f2c(*datatype), *root, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_BCAST(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(buffer, count, datatype, root, comm, ierr);
}

_EXTERN_C_ void mpi_bcast(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(buffer, count, datatype, root, comm, ierr);
}

_EXTERN_C_ void mpi_bcast_(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(buffer, count, datatype, root, comm, ierr);
}

_EXTERN_C_ void mpi_bcast__(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(buffer, count, datatype, root, comm, ierr);
}

/* ================= End Wrappers for MPI_Bcast ================= */


/* ================== C Wrappers for MPI_Bsend ================== */
_EXTERN_C_ int PMPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Bsend(buf, count, datatype, dest, tag, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Bsend =============== */
static void MPI_Bsend_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Bsend((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Bsend((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_BSEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_bsend(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_bsend_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_bsend__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Bsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

/* ================= End Wrappers for MPI_Bsend ================= */


/* ================== C Wrappers for MPI_Bsend_init ================== */
_EXTERN_C_ int PMPI_Bsend_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Bsend_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Bsend_init(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Bsend_init =============== */
static void MPI_Bsend_init_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Bsend_init((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Bsend_init((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_BSEND_INIT(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Bsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_bsend_init(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Bsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_bsend_init_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Bsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_bsend_init__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Bsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Bsend_init ================= */


/* ================== C Wrappers for MPI_Buffer_attach ================== */
_EXTERN_C_ int PMPI_Buffer_attach(void *buffer, int size);
_EXTERN_C_ int MPI_Buffer_attach(void *buffer, int size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Buffer_attach(buffer, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Buffer_attach =============== */
static void MPI_Buffer_attach_fortran_wrapper(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Buffer_attach((void*)buffer, *size);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_BUFFER_ATTACH(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_attach_fortran_wrapper(buffer, size, ierr);
}

_EXTERN_C_ void mpi_buffer_attach(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_attach_fortran_wrapper(buffer, size, ierr);
}

_EXTERN_C_ void mpi_buffer_attach_(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_attach_fortran_wrapper(buffer, size, ierr);
}

_EXTERN_C_ void mpi_buffer_attach__(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_attach_fortran_wrapper(buffer, size, ierr);
}

/* ================= End Wrappers for MPI_Buffer_attach ================= */


/* ================== C Wrappers for MPI_Buffer_detach ================== */
_EXTERN_C_ int PMPI_Buffer_detach(void *buffer, int *size);
_EXTERN_C_ int MPI_Buffer_detach(void *buffer, int *size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Buffer_detach(buffer, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Buffer_detach =============== */
static void MPI_Buffer_detach_fortran_wrapper(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Buffer_detach((void*)buffer, (int*)size);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_BUFFER_DETACH(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_detach_fortran_wrapper(buffer, size, ierr);
}

_EXTERN_C_ void mpi_buffer_detach(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_detach_fortran_wrapper(buffer, size, ierr);
}

_EXTERN_C_ void mpi_buffer_detach_(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_detach_fortran_wrapper(buffer, size, ierr);
}

_EXTERN_C_ void mpi_buffer_detach__(MPI_Fint *buffer, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Buffer_detach_fortran_wrapper(buffer, size, ierr);
}

/* ================= End Wrappers for MPI_Buffer_detach ================= */


/* ================== C Wrappers for MPI_Cancel ================== */
_EXTERN_C_ int PMPI_Cancel(MPI_Request *request);
_EXTERN_C_ int MPI_Cancel(MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Cancel(request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cancel =============== */
static void MPI_Cancel_fortran_wrapper(MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cancel((MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Cancel(&temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CANCEL(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Cancel_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_cancel(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Cancel_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_cancel_(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Cancel_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_cancel__(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Cancel_fortran_wrapper(request, ierr);
}

/* ================= End Wrappers for MPI_Cancel ================= */


/* ================== C Wrappers for MPI_Cart_coords ================== */
_EXTERN_C_ int PMPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]);
_EXTERN_C_ int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cart_coords(comm, rank, maxdims, coords);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_coords =============== */
static void MPI_Cart_coords_fortran_wrapper(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxdims, MPI_Fint coords[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_coords((MPI_Comm)(*comm), *rank, *maxdims, (int*)coords);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Cart_coords(MPI_Comm_f2c(*comm), *rank, *maxdims, (int*)coords);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_COORDS(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxdims, MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_coords_fortran_wrapper(comm, rank, maxdims, coords, ierr);
}

_EXTERN_C_ void mpi_cart_coords(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxdims, MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_coords_fortran_wrapper(comm, rank, maxdims, coords, ierr);
}

_EXTERN_C_ void mpi_cart_coords_(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxdims, MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_coords_fortran_wrapper(comm, rank, maxdims, coords, ierr);
}

_EXTERN_C_ void mpi_cart_coords__(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxdims, MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_coords_fortran_wrapper(comm, rank, maxdims, coords, ierr);
}

/* ================= End Wrappers for MPI_Cart_coords ================= */


/* ================== C Wrappers for MPI_Cart_create ================== */
_EXTERN_C_ int PMPI_Cart_create(MPI_Comm old_comm, int ndims, const int dims[], const int periods[], int reorder, MPI_Comm *comm_cart);
_EXTERN_C_ int MPI_Cart_create(MPI_Comm old_comm, int ndims, const int dims[], const int periods[], int reorder, MPI_Comm *comm_cart) { 
    int _wrap_py_return_val = 0;
{
   swap_world(old_comm);

   _wrap_py_return_val = PMPI_Cart_create(old_comm, ndims, dims, periods, reorder, comm_cart);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_create =============== */
static void MPI_Cart_create_fortran_wrapper(MPI_Fint *old_comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *reorder, MPI_Fint *comm_cart, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_create((MPI_Comm)(*old_comm), *ndims, (const int*)dims, (const int*)periods, *reorder, (MPI_Comm*)comm_cart);
#else /* MPI-2 safe call */
    MPI_Comm temp_comm_cart;
    temp_comm_cart = MPI_Comm_f2c(*comm_cart);
    _wrap_py_return_val = MPI_Cart_create(MPI_Comm_f2c(*old_comm), *ndims, (const int*)dims, (const int*)periods, *reorder, &temp_comm_cart);
    *comm_cart = MPI_Comm_c2f(temp_comm_cart);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_CREATE(MPI_Fint *old_comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *reorder, MPI_Fint *comm_cart, MPI_Fint *ierr) { 
    MPI_Cart_create_fortran_wrapper(old_comm, ndims, dims, periods, reorder, comm_cart, ierr);
}

_EXTERN_C_ void mpi_cart_create(MPI_Fint *old_comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *reorder, MPI_Fint *comm_cart, MPI_Fint *ierr) { 
    MPI_Cart_create_fortran_wrapper(old_comm, ndims, dims, periods, reorder, comm_cart, ierr);
}

_EXTERN_C_ void mpi_cart_create_(MPI_Fint *old_comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *reorder, MPI_Fint *comm_cart, MPI_Fint *ierr) { 
    MPI_Cart_create_fortran_wrapper(old_comm, ndims, dims, periods, reorder, comm_cart, ierr);
}

_EXTERN_C_ void mpi_cart_create__(MPI_Fint *old_comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *reorder, MPI_Fint *comm_cart, MPI_Fint *ierr) { 
    MPI_Cart_create_fortran_wrapper(old_comm, ndims, dims, periods, reorder, comm_cart, ierr);
}

/* ================= End Wrappers for MPI_Cart_create ================= */


/* ================== C Wrappers for MPI_Cart_get ================== */
_EXTERN_C_ int PMPI_Cart_get(MPI_Comm comm, int maxdims, int dims[], int periods[], int coords[]);
_EXTERN_C_ int MPI_Cart_get(MPI_Comm comm, int maxdims, int dims[], int periods[], int coords[]) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cart_get(comm, maxdims, dims, periods, coords);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_get =============== */
static void MPI_Cart_get_fortran_wrapper(MPI_Fint *comm, MPI_Fint *maxdims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint coords[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_get((MPI_Comm)(*comm), *maxdims, (int*)dims, (int*)periods, (int*)coords);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Cart_get(MPI_Comm_f2c(*comm), *maxdims, (int*)dims, (int*)periods, (int*)coords);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_GET(MPI_Fint *comm, MPI_Fint *maxdims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_get_fortran_wrapper(comm, maxdims, dims, periods, coords, ierr);
}

_EXTERN_C_ void mpi_cart_get(MPI_Fint *comm, MPI_Fint *maxdims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_get_fortran_wrapper(comm, maxdims, dims, periods, coords, ierr);
}

_EXTERN_C_ void mpi_cart_get_(MPI_Fint *comm, MPI_Fint *maxdims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_get_fortran_wrapper(comm, maxdims, dims, periods, coords, ierr);
}

_EXTERN_C_ void mpi_cart_get__(MPI_Fint *comm, MPI_Fint *maxdims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint coords[], MPI_Fint *ierr) { 
    MPI_Cart_get_fortran_wrapper(comm, maxdims, dims, periods, coords, ierr);
}

/* ================= End Wrappers for MPI_Cart_get ================= */


/* ================== C Wrappers for MPI_Cart_map ================== */
_EXTERN_C_ int PMPI_Cart_map(MPI_Comm comm, int ndims, const int dims[], const int periods[], int *newrank);
_EXTERN_C_ int MPI_Cart_map(MPI_Comm comm, int ndims, const int dims[], const int periods[], int *newrank) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cart_map(comm, ndims, dims, periods, newrank);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_map =============== */
static void MPI_Cart_map_fortran_wrapper(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_map((MPI_Comm)(*comm), *ndims, (const int*)dims, (const int*)periods, (int*)newrank);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Cart_map(MPI_Comm_f2c(*comm), *ndims, (const int*)dims, (const int*)periods, (int*)newrank);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_MAP(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Cart_map_fortran_wrapper(comm, ndims, dims, periods, newrank, ierr);
}

_EXTERN_C_ void mpi_cart_map(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Cart_map_fortran_wrapper(comm, ndims, dims, periods, newrank, ierr);
}

_EXTERN_C_ void mpi_cart_map_(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Cart_map_fortran_wrapper(comm, ndims, dims, periods, newrank, ierr);
}

_EXTERN_C_ void mpi_cart_map__(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint periods[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Cart_map_fortran_wrapper(comm, ndims, dims, periods, newrank, ierr);
}

/* ================= End Wrappers for MPI_Cart_map ================= */


/* ================== C Wrappers for MPI_Cart_rank ================== */
_EXTERN_C_ int PMPI_Cart_rank(MPI_Comm comm, const int coords[], int *rank);
_EXTERN_C_ int MPI_Cart_rank(MPI_Comm comm, const int coords[], int *rank) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cart_rank(comm, coords, rank);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_rank =============== */
static void MPI_Cart_rank_fortran_wrapper(MPI_Fint *comm, MPI_Fint coords[], MPI_Fint *rank, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_rank((MPI_Comm)(*comm), (const int*)coords, (int*)rank);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Cart_rank(MPI_Comm_f2c(*comm), (const int*)coords, (int*)rank);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_RANK(MPI_Fint *comm, MPI_Fint coords[], MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Cart_rank_fortran_wrapper(comm, coords, rank, ierr);
}

_EXTERN_C_ void mpi_cart_rank(MPI_Fint *comm, MPI_Fint coords[], MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Cart_rank_fortran_wrapper(comm, coords, rank, ierr);
}

_EXTERN_C_ void mpi_cart_rank_(MPI_Fint *comm, MPI_Fint coords[], MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Cart_rank_fortran_wrapper(comm, coords, rank, ierr);
}

_EXTERN_C_ void mpi_cart_rank__(MPI_Fint *comm, MPI_Fint coords[], MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Cart_rank_fortran_wrapper(comm, coords, rank, ierr);
}

/* ================= End Wrappers for MPI_Cart_rank ================= */


/* ================== C Wrappers for MPI_Cart_shift ================== */
_EXTERN_C_ int PMPI_Cart_shift(MPI_Comm comm, int direction, int disp, int *rank_source, int *rank_dest);
_EXTERN_C_ int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int *rank_source, int *rank_dest) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cart_shift(comm, direction, disp, rank_source, rank_dest);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_shift =============== */
static void MPI_Cart_shift_fortran_wrapper(MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_shift((MPI_Comm)(*comm), *direction, *disp, (int*)rank_source, (int*)rank_dest);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Cart_shift(MPI_Comm_f2c(*comm), *direction, *disp, (int*)rank_source, (int*)rank_dest);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_SHIFT(MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr) { 
    MPI_Cart_shift_fortran_wrapper(comm, direction, disp, rank_source, rank_dest, ierr);
}

_EXTERN_C_ void mpi_cart_shift(MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr) { 
    MPI_Cart_shift_fortran_wrapper(comm, direction, disp, rank_source, rank_dest, ierr);
}

_EXTERN_C_ void mpi_cart_shift_(MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr) { 
    MPI_Cart_shift_fortran_wrapper(comm, direction, disp, rank_source, rank_dest, ierr);
}

_EXTERN_C_ void mpi_cart_shift__(MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr) { 
    MPI_Cart_shift_fortran_wrapper(comm, direction, disp, rank_source, rank_dest, ierr);
}

/* ================= End Wrappers for MPI_Cart_shift ================= */


/* ================== C Wrappers for MPI_Cart_sub ================== */
_EXTERN_C_ int PMPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm *new_comm);
_EXTERN_C_ int MPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm *new_comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cart_sub(comm, remain_dims, new_comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cart_sub =============== */
static void MPI_Cart_sub_fortran_wrapper(MPI_Fint *comm, MPI_Fint remain_dims[], MPI_Fint *new_comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cart_sub((MPI_Comm)(*comm), (const int*)remain_dims, (MPI_Comm*)new_comm);
#else /* MPI-2 safe call */
    MPI_Comm temp_new_comm;
    temp_new_comm = MPI_Comm_f2c(*new_comm);
    _wrap_py_return_val = MPI_Cart_sub(MPI_Comm_f2c(*comm), (const int*)remain_dims, &temp_new_comm);
    *new_comm = MPI_Comm_c2f(temp_new_comm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CART_SUB(MPI_Fint *comm, MPI_Fint remain_dims[], MPI_Fint *new_comm, MPI_Fint *ierr) { 
    MPI_Cart_sub_fortran_wrapper(comm, remain_dims, new_comm, ierr);
}

_EXTERN_C_ void mpi_cart_sub(MPI_Fint *comm, MPI_Fint remain_dims[], MPI_Fint *new_comm, MPI_Fint *ierr) { 
    MPI_Cart_sub_fortran_wrapper(comm, remain_dims, new_comm, ierr);
}

_EXTERN_C_ void mpi_cart_sub_(MPI_Fint *comm, MPI_Fint remain_dims[], MPI_Fint *new_comm, MPI_Fint *ierr) { 
    MPI_Cart_sub_fortran_wrapper(comm, remain_dims, new_comm, ierr);
}

_EXTERN_C_ void mpi_cart_sub__(MPI_Fint *comm, MPI_Fint remain_dims[], MPI_Fint *new_comm, MPI_Fint *ierr) { 
    MPI_Cart_sub_fortran_wrapper(comm, remain_dims, new_comm, ierr);
}

/* ================= End Wrappers for MPI_Cart_sub ================= */


/* ================== C Wrappers for MPI_Cartdim_get ================== */
_EXTERN_C_ int PMPI_Cartdim_get(MPI_Comm comm, int *ndims);
_EXTERN_C_ int MPI_Cartdim_get(MPI_Comm comm, int *ndims) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Cartdim_get(comm, ndims);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Cartdim_get =============== */
static void MPI_Cartdim_get_fortran_wrapper(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Cartdim_get((MPI_Comm)(*comm), (int*)ndims);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Cartdim_get(MPI_Comm_f2c(*comm), (int*)ndims);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CARTDIM_GET(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint *ierr) { 
    MPI_Cartdim_get_fortran_wrapper(comm, ndims, ierr);
}

_EXTERN_C_ void mpi_cartdim_get(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint *ierr) { 
    MPI_Cartdim_get_fortran_wrapper(comm, ndims, ierr);
}

_EXTERN_C_ void mpi_cartdim_get_(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint *ierr) { 
    MPI_Cartdim_get_fortran_wrapper(comm, ndims, ierr);
}

_EXTERN_C_ void mpi_cartdim_get__(MPI_Fint *comm, MPI_Fint *ndims, MPI_Fint *ierr) { 
    MPI_Cartdim_get_fortran_wrapper(comm, ndims, ierr);
}

/* ================= End Wrappers for MPI_Cartdim_get ================= */


/* ================== C Wrappers for MPI_Close_port ================== */
_EXTERN_C_ int PMPI_Close_port(const char *port_name);
_EXTERN_C_ int MPI_Close_port(const char *port_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Close_port(port_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Close_port =============== */
static void MPI_Close_port_fortran_wrapper(MPI_Fint *port_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Close_port((const char*)port_name);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_CLOSE_PORT(MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Close_port_fortran_wrapper(port_name, ierr);
}

_EXTERN_C_ void mpi_close_port(MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Close_port_fortran_wrapper(port_name, ierr);
}

_EXTERN_C_ void mpi_close_port_(MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Close_port_fortran_wrapper(port_name, ierr);
}

_EXTERN_C_ void mpi_close_port__(MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Close_port_fortran_wrapper(port_name, ierr);
}

/* ================= End Wrappers for MPI_Close_port ================= */


/* ================== C Wrappers for MPI_Comm_accept ================== */
_EXTERN_C_ int PMPI_Comm_accept(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_accept(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_accept(port_name, info, root, comm, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_accept =============== */
static void MPI_Comm_accept_fortran_wrapper(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_accept((const char*)port_name, (MPI_Info)(*info), *root, (MPI_Comm)(*comm), (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_accept((const char*)port_name, MPI_Info_f2c(*info), *root, MPI_Comm_f2c(*comm), &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_ACCEPT(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_accept_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_accept(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_accept_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_accept_(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_accept_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_accept__(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_accept_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_accept ================= */


/* ================== C Wrappers for MPI_Comm_call_errhandler ================== */
_EXTERN_C_ int PMPI_Comm_call_errhandler(MPI_Comm comm, int errorcode);
_EXTERN_C_ int MPI_Comm_call_errhandler(MPI_Comm comm, int errorcode) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_call_errhandler(comm, errorcode);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_call_errhandler =============== */
static void MPI_Comm_call_errhandler_fortran_wrapper(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_call_errhandler((MPI_Comm)(*comm), *errorcode);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_call_errhandler(MPI_Comm_f2c(*comm), *errorcode);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_CALL_ERRHANDLER(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Comm_call_errhandler_fortran_wrapper(comm, errorcode, ierr);
}

_EXTERN_C_ void mpi_comm_call_errhandler(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Comm_call_errhandler_fortran_wrapper(comm, errorcode, ierr);
}

_EXTERN_C_ void mpi_comm_call_errhandler_(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Comm_call_errhandler_fortran_wrapper(comm, errorcode, ierr);
}

_EXTERN_C_ void mpi_comm_call_errhandler__(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Comm_call_errhandler_fortran_wrapper(comm, errorcode, ierr);
}

/* ================= End Wrappers for MPI_Comm_call_errhandler ================= */


/* ================== C Wrappers for MPI_Comm_compare ================== */
_EXTERN_C_ int PMPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result);
_EXTERN_C_ int MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm1);
swap_world(comm2);

   _wrap_py_return_val = PMPI_Comm_compare(comm1, comm2, result);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_compare =============== */
static void MPI_Comm_compare_fortran_wrapper(MPI_Fint *comm1, MPI_Fint *comm2, MPI_Fint *result, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_compare((MPI_Comm)(*comm1), (MPI_Comm)(*comm2), (int*)result);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_compare(MPI_Comm_f2c(*comm1), MPI_Comm_f2c(*comm2), (int*)result);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_COMPARE(MPI_Fint *comm1, MPI_Fint *comm2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Comm_compare_fortran_wrapper(comm1, comm2, result, ierr);
}

_EXTERN_C_ void mpi_comm_compare(MPI_Fint *comm1, MPI_Fint *comm2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Comm_compare_fortran_wrapper(comm1, comm2, result, ierr);
}

_EXTERN_C_ void mpi_comm_compare_(MPI_Fint *comm1, MPI_Fint *comm2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Comm_compare_fortran_wrapper(comm1, comm2, result, ierr);
}

_EXTERN_C_ void mpi_comm_compare__(MPI_Fint *comm1, MPI_Fint *comm2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Comm_compare_fortran_wrapper(comm1, comm2, result, ierr);
}

/* ================= End Wrappers for MPI_Comm_compare ================= */


/* ================== C Wrappers for MPI_Comm_connect ================== */
_EXTERN_C_ int PMPI_Comm_connect(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_connect(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_connect(port_name, info, root, comm, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_connect =============== */
static void MPI_Comm_connect_fortran_wrapper(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_connect((const char*)port_name, (MPI_Info)(*info), *root, (MPI_Comm)(*comm), (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_connect((const char*)port_name, MPI_Info_f2c(*info), *root, MPI_Comm_f2c(*comm), &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_CONNECT(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_connect_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_connect(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_connect_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_connect_(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_connect_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_connect__(MPI_Fint *port_name, MPI_Fint *info, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_connect_fortran_wrapper(port_name, info, root, comm, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_connect ================= */


/* ================== C Wrappers for MPI_Comm_create ================== */
_EXTERN_C_ int PMPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_create(comm, group, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_create =============== */
static void MPI_Comm_create_fortran_wrapper(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_create((MPI_Comm)(*comm), (MPI_Group)(*group), (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_create(MPI_Comm_f2c(*comm), MPI_Group_f2c(*group), &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_CREATE(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_fortran_wrapper(comm, group, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_create(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_fortran_wrapper(comm, group, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_create_(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_fortran_wrapper(comm, group, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_create__(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_fortran_wrapper(comm, group, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_create ================= */


/* ================== C Wrappers for MPI_Comm_create_errhandler ================== */
_EXTERN_C_ int PMPI_Comm_create_errhandler(MPI_Comm_errhandler_function *function, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_Comm_create_errhandler(MPI_Comm_errhandler_function *function, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_create_errhandler(function, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_create_errhandler =============== */
static void MPI_Comm_create_errhandler_fortran_wrapper(MPI_Comm_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_create_errhandler((MPI_Comm_errhandler_function*)function, (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_Comm_create_errhandler((MPI_Comm_errhandler_function*)function, &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_CREATE_ERRHANDLER(MPI_Comm_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_comm_create_errhandler(MPI_Comm_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_comm_create_errhandler_(MPI_Comm_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_comm_create_errhandler__(MPI_Comm_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Comm_create_errhandler ================= */


/* ================== C Wrappers for MPI_Comm_create_group ================== */
_EXTERN_C_ int PMPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_create_group(comm, group, tag, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_create_group =============== */
static void MPI_Comm_create_group_fortran_wrapper(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *tag, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_create_group((MPI_Comm)(*comm), (MPI_Group)(*group), *tag, (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_create_group(MPI_Comm_f2c(*comm), MPI_Group_f2c(*group), *tag, &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_CREATE_GROUP(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *tag, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_group_fortran_wrapper(comm, group, tag, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_create_group(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *tag, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_group_fortran_wrapper(comm, group, tag, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_create_group_(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *tag, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_group_fortran_wrapper(comm, group, tag, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_create_group__(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *tag, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_create_group_fortran_wrapper(comm, group, tag, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_create_group ================= */


/* ================== C Wrappers for MPI_Comm_create_keyval ================== */
_EXTERN_C_ int PMPI_Comm_create_keyval(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, int *comm_keyval, void *extra_state);
_EXTERN_C_ int MPI_Comm_create_keyval(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, int *comm_keyval, void *extra_state) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_create_keyval(comm_copy_attr_fn, comm_delete_attr_fn, comm_keyval, extra_state);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_create_keyval =============== */
static void MPI_Comm_create_keyval_fortran_wrapper(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, MPI_Fint *comm_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Comm_create_keyval((MPI_Comm_copy_attr_function*)comm_copy_attr_fn, (MPI_Comm_delete_attr_function*)comm_delete_attr_fn, (int*)comm_keyval, (void*)extra_state);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_CREATE_KEYVAL(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, MPI_Fint *comm_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Comm_create_keyval_fortran_wrapper(comm_copy_attr_fn, comm_delete_attr_fn, comm_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_comm_create_keyval(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, MPI_Fint *comm_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Comm_create_keyval_fortran_wrapper(comm_copy_attr_fn, comm_delete_attr_fn, comm_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_comm_create_keyval_(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, MPI_Fint *comm_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Comm_create_keyval_fortran_wrapper(comm_copy_attr_fn, comm_delete_attr_fn, comm_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_comm_create_keyval__(MPI_Comm_copy_attr_function *comm_copy_attr_fn, MPI_Comm_delete_attr_function *comm_delete_attr_fn, MPI_Fint *comm_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Comm_create_keyval_fortran_wrapper(comm_copy_attr_fn, comm_delete_attr_fn, comm_keyval, extra_state, ierr);
}

/* ================= End Wrappers for MPI_Comm_create_keyval ================= */


/* ================== C Wrappers for MPI_Comm_delete_attr ================== */
_EXTERN_C_ int PMPI_Comm_delete_attr(MPI_Comm comm, int comm_keyval);
_EXTERN_C_ int MPI_Comm_delete_attr(MPI_Comm comm, int comm_keyval) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_delete_attr(comm, comm_keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_delete_attr =============== */
static void MPI_Comm_delete_attr_fortran_wrapper(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_delete_attr((MPI_Comm)(*comm), *comm_keyval);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_delete_attr(MPI_Comm_f2c(*comm), *comm_keyval);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_DELETE_ATTR(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_delete_attr_fortran_wrapper(comm, comm_keyval, ierr);
}

_EXTERN_C_ void mpi_comm_delete_attr(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_delete_attr_fortran_wrapper(comm, comm_keyval, ierr);
}

_EXTERN_C_ void mpi_comm_delete_attr_(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_delete_attr_fortran_wrapper(comm, comm_keyval, ierr);
}

_EXTERN_C_ void mpi_comm_delete_attr__(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_delete_attr_fortran_wrapper(comm, comm_keyval, ierr);
}

/* ================= End Wrappers for MPI_Comm_delete_attr ================= */


/* ================== C Wrappers for MPI_Comm_disconnect ================== */
_EXTERN_C_ int PMPI_Comm_disconnect(MPI_Comm *comm);
_EXTERN_C_ int MPI_Comm_disconnect(MPI_Comm *comm) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_disconnect(comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_disconnect =============== */
static void MPI_Comm_disconnect_fortran_wrapper(MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_disconnect((MPI_Comm*)comm);
#else /* MPI-2 safe call */
    MPI_Comm temp_comm;
    temp_comm = MPI_Comm_f2c(*comm);
    _wrap_py_return_val = MPI_Comm_disconnect(&temp_comm);
    *comm = MPI_Comm_c2f(temp_comm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_DISCONNECT(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_disconnect_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_comm_disconnect(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_disconnect_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_comm_disconnect_(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_disconnect_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_comm_disconnect__(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_disconnect_fortran_wrapper(comm, ierr);
}

/* ================= End Wrappers for MPI_Comm_disconnect ================= */


/* ================== C Wrappers for MPI_Comm_dup ================== */
_EXTERN_C_ int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_dup(comm, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_dup =============== */
static void MPI_Comm_dup_fortran_wrapper(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_dup((MPI_Comm)(*comm), (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_dup(MPI_Comm_f2c(*comm), &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_DUP(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_fortran_wrapper(comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_dup(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_fortran_wrapper(comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_dup_(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_fortran_wrapper(comm, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_dup__(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_fortran_wrapper(comm, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_dup ================= */


/* ================== C Wrappers for MPI_Comm_dup_with_info ================== */
_EXTERN_C_ int PMPI_Comm_dup_with_info(MPI_Comm comm, MPI_Info info, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_dup_with_info(MPI_Comm comm, MPI_Info info, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_dup_with_info(comm, info, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_dup_with_info =============== */
static void MPI_Comm_dup_with_info_fortran_wrapper(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_dup_with_info((MPI_Comm)(*comm), (MPI_Info)(*info), (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_dup_with_info(MPI_Comm_f2c(*comm), MPI_Info_f2c(*info), &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_DUP_WITH_INFO(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_with_info_fortran_wrapper(comm, info, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_dup_with_info(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_with_info_fortran_wrapper(comm, info, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_dup_with_info_(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_with_info_fortran_wrapper(comm, info, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_dup_with_info__(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_dup_with_info_fortran_wrapper(comm, info, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_dup_with_info ================= */


/* ================== C Wrappers for MPI_Comm_free ================== */
_EXTERN_C_ int PMPI_Comm_free(MPI_Comm *comm);
_EXTERN_C_ int MPI_Comm_free(MPI_Comm *comm) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_free(comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_free =============== */
static void MPI_Comm_free_fortran_wrapper(MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_free((MPI_Comm*)comm);
#else /* MPI-2 safe call */
    MPI_Comm temp_comm;
    temp_comm = MPI_Comm_f2c(*comm);
    _wrap_py_return_val = MPI_Comm_free(&temp_comm);
    *comm = MPI_Comm_c2f(temp_comm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_FREE(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_free_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_comm_free(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_free_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_comm_free_(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_free_fortran_wrapper(comm, ierr);
}

_EXTERN_C_ void mpi_comm_free__(MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Comm_free_fortran_wrapper(comm, ierr);
}

/* ================= End Wrappers for MPI_Comm_free ================= */


/* ================== C Wrappers for MPI_Comm_free_keyval ================== */
_EXTERN_C_ int PMPI_Comm_free_keyval(int *comm_keyval);
_EXTERN_C_ int MPI_Comm_free_keyval(int *comm_keyval) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_free_keyval(comm_keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_free_keyval =============== */
static void MPI_Comm_free_keyval_fortran_wrapper(MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Comm_free_keyval((int*)comm_keyval);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_FREE_KEYVAL(MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_free_keyval_fortran_wrapper(comm_keyval, ierr);
}

_EXTERN_C_ void mpi_comm_free_keyval(MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_free_keyval_fortran_wrapper(comm_keyval, ierr);
}

_EXTERN_C_ void mpi_comm_free_keyval_(MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_free_keyval_fortran_wrapper(comm_keyval, ierr);
}

_EXTERN_C_ void mpi_comm_free_keyval__(MPI_Fint *comm_keyval, MPI_Fint *ierr) { 
    MPI_Comm_free_keyval_fortran_wrapper(comm_keyval, ierr);
}

/* ================= End Wrappers for MPI_Comm_free_keyval ================= */


/* ================== C Wrappers for MPI_Comm_get_attr ================== */
_EXTERN_C_ int PMPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);
_EXTERN_C_ int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_get_attr(comm, comm_keyval, attribute_val, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_get_attr =============== */
static void MPI_Comm_get_attr_fortran_wrapper(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_get_attr((MPI_Comm)(*comm), *comm_keyval, (void*)attribute_val, (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_get_attr(MPI_Comm_f2c(*comm), *comm_keyval, (void*)attribute_val, (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_GET_ATTR(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_get_attr_fortran_wrapper(comm, comm_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_comm_get_attr(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_get_attr_fortran_wrapper(comm, comm_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_comm_get_attr_(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_get_attr_fortran_wrapper(comm, comm_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_comm_get_attr__(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_get_attr_fortran_wrapper(comm, comm_keyval, attribute_val, flag, ierr);
}

/* ================= End Wrappers for MPI_Comm_get_attr ================= */


/* ================== C Wrappers for MPI_Comm_get_errhandler ================== */
_EXTERN_C_ int PMPI_Comm_get_errhandler(MPI_Comm comm, MPI_Errhandler *erhandler);
_EXTERN_C_ int MPI_Comm_get_errhandler(MPI_Comm comm, MPI_Errhandler *erhandler) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_get_errhandler(comm, erhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_get_errhandler =============== */
static void MPI_Comm_get_errhandler_fortran_wrapper(MPI_Fint *comm, MPI_Fint *erhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_get_errhandler((MPI_Comm)(*comm), (MPI_Errhandler*)erhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_erhandler;
    temp_erhandler = MPI_Errhandler_f2c(*erhandler);
    _wrap_py_return_val = MPI_Comm_get_errhandler(MPI_Comm_f2c(*comm), &temp_erhandler);
    *erhandler = MPI_Errhandler_c2f(temp_erhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_GET_ERRHANDLER(MPI_Fint *comm, MPI_Fint *erhandler, MPI_Fint *ierr) { 
    MPI_Comm_get_errhandler_fortran_wrapper(comm, erhandler, ierr);
}

_EXTERN_C_ void mpi_comm_get_errhandler(MPI_Fint *comm, MPI_Fint *erhandler, MPI_Fint *ierr) { 
    MPI_Comm_get_errhandler_fortran_wrapper(comm, erhandler, ierr);
}

_EXTERN_C_ void mpi_comm_get_errhandler_(MPI_Fint *comm, MPI_Fint *erhandler, MPI_Fint *ierr) { 
    MPI_Comm_get_errhandler_fortran_wrapper(comm, erhandler, ierr);
}

_EXTERN_C_ void mpi_comm_get_errhandler__(MPI_Fint *comm, MPI_Fint *erhandler, MPI_Fint *ierr) { 
    MPI_Comm_get_errhandler_fortran_wrapper(comm, erhandler, ierr);
}

/* ================= End Wrappers for MPI_Comm_get_errhandler ================= */


/* ================== C Wrappers for MPI_Comm_get_info ================== */
_EXTERN_C_ int PMPI_Comm_get_info(MPI_Comm comm, MPI_Info *info_used);
_EXTERN_C_ int MPI_Comm_get_info(MPI_Comm comm, MPI_Info *info_used) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_get_info(comm, info_used);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_get_info =============== */
static void MPI_Comm_get_info_fortran_wrapper(MPI_Fint *comm, MPI_Fint *info_used, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_get_info((MPI_Comm)(*comm), (MPI_Info*)info_used);
#else /* MPI-2 safe call */
    MPI_Info temp_info_used;
    temp_info_used = MPI_Info_f2c(*info_used);
    _wrap_py_return_val = MPI_Comm_get_info(MPI_Comm_f2c(*comm), &temp_info_used);
    *info_used = MPI_Info_c2f(temp_info_used);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_GET_INFO(MPI_Fint *comm, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Comm_get_info_fortran_wrapper(comm, info_used, ierr);
}

_EXTERN_C_ void mpi_comm_get_info(MPI_Fint *comm, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Comm_get_info_fortran_wrapper(comm, info_used, ierr);
}

_EXTERN_C_ void mpi_comm_get_info_(MPI_Fint *comm, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Comm_get_info_fortran_wrapper(comm, info_used, ierr);
}

_EXTERN_C_ void mpi_comm_get_info__(MPI_Fint *comm, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Comm_get_info_fortran_wrapper(comm, info_used, ierr);
}

/* ================= End Wrappers for MPI_Comm_get_info ================= */


/* ================== C Wrappers for MPI_Comm_get_name ================== */
_EXTERN_C_ int PMPI_Comm_get_name(MPI_Comm comm, char *comm_name, int *resultlen);
_EXTERN_C_ int MPI_Comm_get_name(MPI_Comm comm, char *comm_name, int *resultlen) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_get_name(comm, comm_name, resultlen);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_get_name =============== */
static void MPI_Comm_get_name_fortran_wrapper(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_get_name((MPI_Comm)(*comm), (char*)comm_name, (int*)resultlen);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_get_name(MPI_Comm_f2c(*comm), (char*)comm_name, (int*)resultlen);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_GET_NAME(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Comm_get_name_fortran_wrapper(comm, comm_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_comm_get_name(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Comm_get_name_fortran_wrapper(comm, comm_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_comm_get_name_(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Comm_get_name_fortran_wrapper(comm, comm_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_comm_get_name__(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Comm_get_name_fortran_wrapper(comm, comm_name, resultlen, ierr);
}

/* ================= End Wrappers for MPI_Comm_get_name ================= */


/* ================== C Wrappers for MPI_Comm_get_parent ================== */
_EXTERN_C_ int PMPI_Comm_get_parent(MPI_Comm *parent);
_EXTERN_C_ int MPI_Comm_get_parent(MPI_Comm *parent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_get_parent(parent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_get_parent =============== */
static void MPI_Comm_get_parent_fortran_wrapper(MPI_Fint *parent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_get_parent((MPI_Comm*)parent);
#else /* MPI-2 safe call */
    MPI_Comm temp_parent;
    temp_parent = MPI_Comm_f2c(*parent);
    _wrap_py_return_val = MPI_Comm_get_parent(&temp_parent);
    *parent = MPI_Comm_c2f(temp_parent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_GET_PARENT(MPI_Fint *parent, MPI_Fint *ierr) { 
    MPI_Comm_get_parent_fortran_wrapper(parent, ierr);
}

_EXTERN_C_ void mpi_comm_get_parent(MPI_Fint *parent, MPI_Fint *ierr) { 
    MPI_Comm_get_parent_fortran_wrapper(parent, ierr);
}

_EXTERN_C_ void mpi_comm_get_parent_(MPI_Fint *parent, MPI_Fint *ierr) { 
    MPI_Comm_get_parent_fortran_wrapper(parent, ierr);
}

_EXTERN_C_ void mpi_comm_get_parent__(MPI_Fint *parent, MPI_Fint *ierr) { 
    MPI_Comm_get_parent_fortran_wrapper(parent, ierr);
}

/* ================= End Wrappers for MPI_Comm_get_parent ================= */


/* ================== C Wrappers for MPI_Comm_group ================== */
_EXTERN_C_ int PMPI_Comm_group(MPI_Comm comm, MPI_Group *group);
_EXTERN_C_ int MPI_Comm_group(MPI_Comm comm, MPI_Group *group) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_group(comm, group);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_group =============== */
static void MPI_Comm_group_fortran_wrapper(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_group((MPI_Comm)(*comm), (MPI_Group*)group);
#else /* MPI-2 safe call */
    MPI_Group temp_group;
    temp_group = MPI_Group_f2c(*group);
    _wrap_py_return_val = MPI_Comm_group(MPI_Comm_f2c(*comm), &temp_group);
    *group = MPI_Group_c2f(temp_group);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_GROUP(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_group_fortran_wrapper(comm, group, ierr);
}

_EXTERN_C_ void mpi_comm_group(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_group_fortran_wrapper(comm, group, ierr);
}

_EXTERN_C_ void mpi_comm_group_(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_group_fortran_wrapper(comm, group, ierr);
}

_EXTERN_C_ void mpi_comm_group__(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_group_fortran_wrapper(comm, group, ierr);
}

/* ================= End Wrappers for MPI_Comm_group ================= */


/* ================== C Wrappers for MPI_Comm_idup ================== */
_EXTERN_C_ int PMPI_Comm_idup(MPI_Comm comm, MPI_Comm *newcomm, MPI_Request *request);
_EXTERN_C_ int MPI_Comm_idup(MPI_Comm comm, MPI_Comm *newcomm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_idup(comm, newcomm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_idup =============== */
static void MPI_Comm_idup_fortran_wrapper(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_idup((MPI_Comm)(*comm), (MPI_Comm*)newcomm, (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Comm_idup(MPI_Comm_f2c(*comm), &temp_newcomm, &temp_request);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_IDUP(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Comm_idup_fortran_wrapper(comm, newcomm, request, ierr);
}

_EXTERN_C_ void mpi_comm_idup(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Comm_idup_fortran_wrapper(comm, newcomm, request, ierr);
}

_EXTERN_C_ void mpi_comm_idup_(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Comm_idup_fortran_wrapper(comm, newcomm, request, ierr);
}

_EXTERN_C_ void mpi_comm_idup__(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Comm_idup_fortran_wrapper(comm, newcomm, request, ierr);
}

/* ================= End Wrappers for MPI_Comm_idup ================= */


/* ================== C Wrappers for MPI_Comm_join ================== */
_EXTERN_C_ int PMPI_Comm_join(int fd, MPI_Comm *intercomm);
_EXTERN_C_ int MPI_Comm_join(int fd, MPI_Comm *intercomm) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Comm_join(fd, intercomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_join =============== */
static void MPI_Comm_join_fortran_wrapper(MPI_Fint *fd, MPI_Fint *intercomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_join(*fd, (MPI_Comm*)intercomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_intercomm;
    temp_intercomm = MPI_Comm_f2c(*intercomm);
    _wrap_py_return_val = MPI_Comm_join(*fd, &temp_intercomm);
    *intercomm = MPI_Comm_c2f(temp_intercomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_JOIN(MPI_Fint *fd, MPI_Fint *intercomm, MPI_Fint *ierr) { 
    MPI_Comm_join_fortran_wrapper(fd, intercomm, ierr);
}

_EXTERN_C_ void mpi_comm_join(MPI_Fint *fd, MPI_Fint *intercomm, MPI_Fint *ierr) { 
    MPI_Comm_join_fortran_wrapper(fd, intercomm, ierr);
}

_EXTERN_C_ void mpi_comm_join_(MPI_Fint *fd, MPI_Fint *intercomm, MPI_Fint *ierr) { 
    MPI_Comm_join_fortran_wrapper(fd, intercomm, ierr);
}

_EXTERN_C_ void mpi_comm_join__(MPI_Fint *fd, MPI_Fint *intercomm, MPI_Fint *ierr) { 
    MPI_Comm_join_fortran_wrapper(fd, intercomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_join ================= */


/* ================== C Wrappers for MPI_Comm_rank ================== */
_EXTERN_C_ int PMPI_Comm_rank(MPI_Comm comm, int *rank);
_EXTERN_C_ int MPI_Comm_rank(MPI_Comm comm, int *rank) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_rank(comm, rank);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_rank =============== */
static void MPI_Comm_rank_fortran_wrapper(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_rank((MPI_Comm)(*comm), (int*)rank);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_rank(MPI_Comm_f2c(*comm), (int*)rank);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_RANK(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Comm_rank_fortran_wrapper(comm, rank, ierr);
}

_EXTERN_C_ void mpi_comm_rank(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Comm_rank_fortran_wrapper(comm, rank, ierr);
}

_EXTERN_C_ void mpi_comm_rank_(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Comm_rank_fortran_wrapper(comm, rank, ierr);
}

_EXTERN_C_ void mpi_comm_rank__(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Comm_rank_fortran_wrapper(comm, rank, ierr);
}

/* ================= End Wrappers for MPI_Comm_rank ================= */


/* ================== C Wrappers for MPI_Comm_remote_group ================== */
_EXTERN_C_ int PMPI_Comm_remote_group(MPI_Comm comm, MPI_Group *group);
_EXTERN_C_ int MPI_Comm_remote_group(MPI_Comm comm, MPI_Group *group) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_remote_group(comm, group);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_remote_group =============== */
static void MPI_Comm_remote_group_fortran_wrapper(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_remote_group((MPI_Comm)(*comm), (MPI_Group*)group);
#else /* MPI-2 safe call */
    MPI_Group temp_group;
    temp_group = MPI_Group_f2c(*group);
    _wrap_py_return_val = MPI_Comm_remote_group(MPI_Comm_f2c(*comm), &temp_group);
    *group = MPI_Group_c2f(temp_group);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_REMOTE_GROUP(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_remote_group_fortran_wrapper(comm, group, ierr);
}

_EXTERN_C_ void mpi_comm_remote_group(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_remote_group_fortran_wrapper(comm, group, ierr);
}

_EXTERN_C_ void mpi_comm_remote_group_(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_remote_group_fortran_wrapper(comm, group, ierr);
}

_EXTERN_C_ void mpi_comm_remote_group__(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Comm_remote_group_fortran_wrapper(comm, group, ierr);
}

/* ================= End Wrappers for MPI_Comm_remote_group ================= */


/* ================== C Wrappers for MPI_Comm_remote_size ================== */
_EXTERN_C_ int PMPI_Comm_remote_size(MPI_Comm comm, int *size);
_EXTERN_C_ int MPI_Comm_remote_size(MPI_Comm comm, int *size) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_remote_size(comm, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_remote_size =============== */
static void MPI_Comm_remote_size_fortran_wrapper(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_remote_size((MPI_Comm)(*comm), (int*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_remote_size(MPI_Comm_f2c(*comm), (int*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_REMOTE_SIZE(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_remote_size_fortran_wrapper(comm, size, ierr);
}

_EXTERN_C_ void mpi_comm_remote_size(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_remote_size_fortran_wrapper(comm, size, ierr);
}

_EXTERN_C_ void mpi_comm_remote_size_(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_remote_size_fortran_wrapper(comm, size, ierr);
}

_EXTERN_C_ void mpi_comm_remote_size__(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_remote_size_fortran_wrapper(comm, size, ierr);
}

/* ================= End Wrappers for MPI_Comm_remote_size ================= */


/* ================== C Wrappers for MPI_Comm_set_attr ================== */
_EXTERN_C_ int PMPI_Comm_set_attr(MPI_Comm comm, int comm_keyval, void *attribute_val);
_EXTERN_C_ int MPI_Comm_set_attr(MPI_Comm comm, int comm_keyval, void *attribute_val) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_set_attr(comm, comm_keyval, attribute_val);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_set_attr =============== */
static void MPI_Comm_set_attr_fortran_wrapper(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_set_attr((MPI_Comm)(*comm), *comm_keyval, (void*)attribute_val);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_set_attr(MPI_Comm_f2c(*comm), *comm_keyval, (void*)attribute_val);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SET_ATTR(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Comm_set_attr_fortran_wrapper(comm, comm_keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_comm_set_attr(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Comm_set_attr_fortran_wrapper(comm, comm_keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_comm_set_attr_(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Comm_set_attr_fortran_wrapper(comm, comm_keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_comm_set_attr__(MPI_Fint *comm, MPI_Fint *comm_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Comm_set_attr_fortran_wrapper(comm, comm_keyval, attribute_val, ierr);
}

/* ================= End Wrappers for MPI_Comm_set_attr ================= */


/* ================== C Wrappers for MPI_Comm_set_errhandler ================== */
_EXTERN_C_ int PMPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler errhandler);
_EXTERN_C_ int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler errhandler) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_set_errhandler(comm, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_set_errhandler =============== */
static void MPI_Comm_set_errhandler_fortran_wrapper(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_set_errhandler((MPI_Comm)(*comm), (MPI_Errhandler)(*errhandler));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_set_errhandler(MPI_Comm_f2c(*comm), MPI_Errhandler_f2c(*errhandler));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SET_ERRHANDLER(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_set_errhandler_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_comm_set_errhandler(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_set_errhandler_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_comm_set_errhandler_(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_set_errhandler_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_comm_set_errhandler__(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Comm_set_errhandler_fortran_wrapper(comm, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Comm_set_errhandler ================= */


/* ================== C Wrappers for MPI_Comm_set_info ================== */
_EXTERN_C_ int PMPI_Comm_set_info(MPI_Comm comm, MPI_Info info);
_EXTERN_C_ int MPI_Comm_set_info(MPI_Comm comm, MPI_Info info) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_set_info(comm, info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_set_info =============== */
static void MPI_Comm_set_info_fortran_wrapper(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_set_info((MPI_Comm)(*comm), (MPI_Info)(*info));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_set_info(MPI_Comm_f2c(*comm), MPI_Info_f2c(*info));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SET_INFO(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Comm_set_info_fortran_wrapper(comm, info, ierr);
}

_EXTERN_C_ void mpi_comm_set_info(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Comm_set_info_fortran_wrapper(comm, info, ierr);
}

_EXTERN_C_ void mpi_comm_set_info_(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Comm_set_info_fortran_wrapper(comm, info, ierr);
}

_EXTERN_C_ void mpi_comm_set_info__(MPI_Fint *comm, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Comm_set_info_fortran_wrapper(comm, info, ierr);
}

/* ================= End Wrappers for MPI_Comm_set_info ================= */


/* ================== C Wrappers for MPI_Comm_set_name ================== */
_EXTERN_C_ int PMPI_Comm_set_name(MPI_Comm comm, const char *comm_name);
_EXTERN_C_ int MPI_Comm_set_name(MPI_Comm comm, const char *comm_name) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_set_name(comm, comm_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_set_name =============== */
static void MPI_Comm_set_name_fortran_wrapper(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_set_name((MPI_Comm)(*comm), (const char*)comm_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_set_name(MPI_Comm_f2c(*comm), (const char*)comm_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SET_NAME(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *ierr) { 
    MPI_Comm_set_name_fortran_wrapper(comm, comm_name, ierr);
}

_EXTERN_C_ void mpi_comm_set_name(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *ierr) { 
    MPI_Comm_set_name_fortran_wrapper(comm, comm_name, ierr);
}

_EXTERN_C_ void mpi_comm_set_name_(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *ierr) { 
    MPI_Comm_set_name_fortran_wrapper(comm, comm_name, ierr);
}

_EXTERN_C_ void mpi_comm_set_name__(MPI_Fint *comm, MPI_Fint *comm_name, MPI_Fint *ierr) { 
    MPI_Comm_set_name_fortran_wrapper(comm, comm_name, ierr);
}

/* ================= End Wrappers for MPI_Comm_set_name ================= */


/* ================== C Wrappers for MPI_Comm_size ================== */
_EXTERN_C_ int PMPI_Comm_size(MPI_Comm comm, int *size);
_EXTERN_C_ int MPI_Comm_size(MPI_Comm comm, int *size) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_size(comm, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_size =============== */
static void MPI_Comm_size_fortran_wrapper(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_size((MPI_Comm)(*comm), (int*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_size(MPI_Comm_f2c(*comm), (int*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SIZE(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_size_fortran_wrapper(comm, size, ierr);
}

_EXTERN_C_ void mpi_comm_size(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_size_fortran_wrapper(comm, size, ierr);
}

_EXTERN_C_ void mpi_comm_size_(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_size_fortran_wrapper(comm, size, ierr);
}

_EXTERN_C_ void mpi_comm_size__(MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Comm_size_fortran_wrapper(comm, size, ierr);
}

/* ================= End Wrappers for MPI_Comm_size ================= */


/* ================== C Wrappers for MPI_Comm_split ================== */
_EXTERN_C_ int PMPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_split(comm, color, key, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_split =============== */
static void MPI_Comm_split_fortran_wrapper(MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_split((MPI_Comm)(*comm), *color, *key, (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_split(MPI_Comm_f2c(*comm), *color, *key, &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SPLIT(MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_fortran_wrapper(comm, color, key, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_split(MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_fortran_wrapper(comm, color, key, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_split_(MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_fortran_wrapper(comm, color, key, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_split__(MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_fortran_wrapper(comm, color, key, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_split ================= */


/* ================== C Wrappers for MPI_Comm_split_type ================== */
_EXTERN_C_ int PMPI_Comm_split_type(MPI_Comm comm, int split_type, int key, MPI_Info info, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Comm_split_type(MPI_Comm comm, int split_type, int key, MPI_Info info, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_split_type(comm, split_type, key, info, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_split_type =============== */
static void MPI_Comm_split_type_fortran_wrapper(MPI_Fint *comm, MPI_Fint *split_type, MPI_Fint *key, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_split_type((MPI_Comm)(*comm), *split_type, *key, (MPI_Info)(*info), (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Comm_split_type(MPI_Comm_f2c(*comm), *split_type, *key, MPI_Info_f2c(*info), &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_SPLIT_TYPE(MPI_Fint *comm, MPI_Fint *split_type, MPI_Fint *key, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_type_fortran_wrapper(comm, split_type, key, info, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_split_type(MPI_Fint *comm, MPI_Fint *split_type, MPI_Fint *key, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_type_fortran_wrapper(comm, split_type, key, info, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_split_type_(MPI_Fint *comm, MPI_Fint *split_type, MPI_Fint *key, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_type_fortran_wrapper(comm, split_type, key, info, newcomm, ierr);
}

_EXTERN_C_ void mpi_comm_split_type__(MPI_Fint *comm, MPI_Fint *split_type, MPI_Fint *key, MPI_Fint *info, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Comm_split_type_fortran_wrapper(comm, split_type, key, info, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Comm_split_type ================= */


/* ================== C Wrappers for MPI_Comm_test_inter ================== */
_EXTERN_C_ int PMPI_Comm_test_inter(MPI_Comm comm, int *flag);
_EXTERN_C_ int MPI_Comm_test_inter(MPI_Comm comm, int *flag) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Comm_test_inter(comm, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Comm_test_inter =============== */
static void MPI_Comm_test_inter_fortran_wrapper(MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Comm_test_inter((MPI_Comm)(*comm), (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Comm_test_inter(MPI_Comm_f2c(*comm), (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMM_TEST_INTER(MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_test_inter_fortran_wrapper(comm, flag, ierr);
}

_EXTERN_C_ void mpi_comm_test_inter(MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_test_inter_fortran_wrapper(comm, flag, ierr);
}

_EXTERN_C_ void mpi_comm_test_inter_(MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_test_inter_fortran_wrapper(comm, flag, ierr);
}

_EXTERN_C_ void mpi_comm_test_inter__(MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Comm_test_inter_fortran_wrapper(comm, flag, ierr);
}

/* ================= End Wrappers for MPI_Comm_test_inter ================= */


/* ================== C Wrappers for MPI_Compare_and_swap ================== */
_EXTERN_C_ int PMPI_Compare_and_swap(const void *origin_addr, const void *compare_addr, void *result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Win win);
_EXTERN_C_ int MPI_Compare_and_swap(const void *origin_addr, const void *compare_addr, void *result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Compare_and_swap(origin_addr, compare_addr, result_addr, datatype, target_rank, target_disp, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Compare_and_swap =============== */
static void MPI_Compare_and_swap_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *compare_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Compare_and_swap((const void*)origin_addr, (const void*)compare_addr, (void*)result_addr, (MPI_Datatype)(*datatype), *target_rank, *target_disp, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Compare_and_swap((const void*)origin_addr, (const void*)compare_addr, (void*)result_addr, MPI_Type_f2c(*datatype), *target_rank, *target_disp, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_COMPARE_AND_SWAP(MPI_Fint *origin_addr, MPI_Fint *compare_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Compare_and_swap_fortran_wrapper(origin_addr, compare_addr, result_addr, datatype, target_rank, target_disp, win, ierr);
}

_EXTERN_C_ void mpi_compare_and_swap(MPI_Fint *origin_addr, MPI_Fint *compare_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Compare_and_swap_fortran_wrapper(origin_addr, compare_addr, result_addr, datatype, target_rank, target_disp, win, ierr);
}

_EXTERN_C_ void mpi_compare_and_swap_(MPI_Fint *origin_addr, MPI_Fint *compare_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Compare_and_swap_fortran_wrapper(origin_addr, compare_addr, result_addr, datatype, target_rank, target_disp, win, ierr);
}

_EXTERN_C_ void mpi_compare_and_swap__(MPI_Fint *origin_addr, MPI_Fint *compare_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Compare_and_swap_fortran_wrapper(origin_addr, compare_addr, result_addr, datatype, target_rank, target_disp, win, ierr);
}

/* ================= End Wrappers for MPI_Compare_and_swap ================= */


/* ================== C Wrappers for MPI_Dims_create ================== */
_EXTERN_C_ int PMPI_Dims_create(int nnodes, int ndims, int dims[]);
_EXTERN_C_ int MPI_Dims_create(int nnodes, int ndims, int dims[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Dims_create(nnodes, ndims, dims);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Dims_create =============== */
static void MPI_Dims_create_fortran_wrapper(MPI_Fint *nnodes, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Dims_create(*nnodes, *ndims, (int*)dims);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_DIMS_CREATE(MPI_Fint *nnodes, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint *ierr) { 
    MPI_Dims_create_fortran_wrapper(nnodes, ndims, dims, ierr);
}

_EXTERN_C_ void mpi_dims_create(MPI_Fint *nnodes, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint *ierr) { 
    MPI_Dims_create_fortran_wrapper(nnodes, ndims, dims, ierr);
}

_EXTERN_C_ void mpi_dims_create_(MPI_Fint *nnodes, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint *ierr) { 
    MPI_Dims_create_fortran_wrapper(nnodes, ndims, dims, ierr);
}

_EXTERN_C_ void mpi_dims_create__(MPI_Fint *nnodes, MPI_Fint *ndims, MPI_Fint dims[], MPI_Fint *ierr) { 
    MPI_Dims_create_fortran_wrapper(nnodes, ndims, dims, ierr);
}

/* ================= End Wrappers for MPI_Dims_create ================= */


/* ================== C Wrappers for MPI_Dist_graph_create ================== */
_EXTERN_C_ int PMPI_Dist_graph_create(MPI_Comm comm_old, int n, const int nodes[], const int degrees[], const int targets[], const int weights[], MPI_Info info, int reorder, MPI_Comm *newcomm);
_EXTERN_C_ int MPI_Dist_graph_create(MPI_Comm comm_old, int n, const int nodes[], const int degrees[], const int targets[], const int weights[], MPI_Info info, int reorder, MPI_Comm *newcomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm_old);

   _wrap_py_return_val = PMPI_Dist_graph_create(comm_old, n, nodes, degrees, targets, weights, info, reorder, newcomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Dist_graph_create =============== */
static void MPI_Dist_graph_create_fortran_wrapper(MPI_Fint *comm_old, MPI_Fint *n, MPI_Fint nodes[], MPI_Fint degrees[], MPI_Fint targets[], MPI_Fint weights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Dist_graph_create((MPI_Comm)(*comm_old), *n, (const int*)nodes, (const int*)degrees, (const int*)targets, (const int*)weights, (MPI_Info)(*info), *reorder, (MPI_Comm*)newcomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newcomm;
    temp_newcomm = MPI_Comm_f2c(*newcomm);
    _wrap_py_return_val = MPI_Dist_graph_create(MPI_Comm_f2c(*comm_old), *n, (const int*)nodes, (const int*)degrees, (const int*)targets, (const int*)weights, MPI_Info_f2c(*info), *reorder, &temp_newcomm);
    *newcomm = MPI_Comm_c2f(temp_newcomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_DIST_GRAPH_CREATE(MPI_Fint *comm_old, MPI_Fint *n, MPI_Fint nodes[], MPI_Fint degrees[], MPI_Fint targets[], MPI_Fint weights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_fortran_wrapper(comm_old, n, nodes, degrees, targets, weights, info, reorder, newcomm, ierr);
}

_EXTERN_C_ void mpi_dist_graph_create(MPI_Fint *comm_old, MPI_Fint *n, MPI_Fint nodes[], MPI_Fint degrees[], MPI_Fint targets[], MPI_Fint weights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_fortran_wrapper(comm_old, n, nodes, degrees, targets, weights, info, reorder, newcomm, ierr);
}

_EXTERN_C_ void mpi_dist_graph_create_(MPI_Fint *comm_old, MPI_Fint *n, MPI_Fint nodes[], MPI_Fint degrees[], MPI_Fint targets[], MPI_Fint weights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_fortran_wrapper(comm_old, n, nodes, degrees, targets, weights, info, reorder, newcomm, ierr);
}

_EXTERN_C_ void mpi_dist_graph_create__(MPI_Fint *comm_old, MPI_Fint *n, MPI_Fint nodes[], MPI_Fint degrees[], MPI_Fint targets[], MPI_Fint weights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *newcomm, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_fortran_wrapper(comm_old, n, nodes, degrees, targets, weights, info, reorder, newcomm, ierr);
}

/* ================= End Wrappers for MPI_Dist_graph_create ================= */


/* ================== C Wrappers for MPI_Dist_graph_create_adjacent ================== */
_EXTERN_C_ int PMPI_Dist_graph_create_adjacent(MPI_Comm comm_old, int indegree, const int sources[], const int sourceweights[], int outdegree, const int destinations[], const int destweights[], MPI_Info info, int reorder, MPI_Comm *comm_dist_graph);
_EXTERN_C_ int MPI_Dist_graph_create_adjacent(MPI_Comm comm_old, int indegree, const int sources[], const int sourceweights[], int outdegree, const int destinations[], const int destweights[], MPI_Info info, int reorder, MPI_Comm *comm_dist_graph) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm_old);

   _wrap_py_return_val = PMPI_Dist_graph_create_adjacent(comm_old, indegree, sources, sourceweights, outdegree, destinations, destweights, info, reorder, comm_dist_graph);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Dist_graph_create_adjacent =============== */
static void MPI_Dist_graph_create_adjacent_fortran_wrapper(MPI_Fint *comm_old, MPI_Fint *indegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *outdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *comm_dist_graph, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Dist_graph_create_adjacent((MPI_Comm)(*comm_old), *indegree, (const int*)sources, (const int*)sourceweights, *outdegree, (const int*)destinations, (const int*)destweights, (MPI_Info)(*info), *reorder, (MPI_Comm*)comm_dist_graph);
#else /* MPI-2 safe call */
    MPI_Comm temp_comm_dist_graph;
    temp_comm_dist_graph = MPI_Comm_f2c(*comm_dist_graph);
    _wrap_py_return_val = MPI_Dist_graph_create_adjacent(MPI_Comm_f2c(*comm_old), *indegree, (const int*)sources, (const int*)sourceweights, *outdegree, (const int*)destinations, (const int*)destweights, MPI_Info_f2c(*info), *reorder, &temp_comm_dist_graph);
    *comm_dist_graph = MPI_Comm_c2f(temp_comm_dist_graph);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_DIST_GRAPH_CREATE_ADJACENT(MPI_Fint *comm_old, MPI_Fint *indegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *outdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *comm_dist_graph, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_adjacent_fortran_wrapper(comm_old, indegree, sources, sourceweights, outdegree, destinations, destweights, info, reorder, comm_dist_graph, ierr);
}

_EXTERN_C_ void mpi_dist_graph_create_adjacent(MPI_Fint *comm_old, MPI_Fint *indegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *outdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *comm_dist_graph, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_adjacent_fortran_wrapper(comm_old, indegree, sources, sourceweights, outdegree, destinations, destweights, info, reorder, comm_dist_graph, ierr);
}

_EXTERN_C_ void mpi_dist_graph_create_adjacent_(MPI_Fint *comm_old, MPI_Fint *indegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *outdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *comm_dist_graph, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_adjacent_fortran_wrapper(comm_old, indegree, sources, sourceweights, outdegree, destinations, destweights, info, reorder, comm_dist_graph, ierr);
}

_EXTERN_C_ void mpi_dist_graph_create_adjacent__(MPI_Fint *comm_old, MPI_Fint *indegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *outdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *info, MPI_Fint *reorder, MPI_Fint *comm_dist_graph, MPI_Fint *ierr) { 
    MPI_Dist_graph_create_adjacent_fortran_wrapper(comm_old, indegree, sources, sourceweights, outdegree, destinations, destweights, info, reorder, comm_dist_graph, ierr);
}

/* ================= End Wrappers for MPI_Dist_graph_create_adjacent ================= */


/* ================== C Wrappers for MPI_Dist_graph_neighbors ================== */
_EXTERN_C_ int PMPI_Dist_graph_neighbors(MPI_Comm comm, int maxindegree, int sources[], int sourceweights[], int maxoutdegree, int destinations[], int destweights[]);
_EXTERN_C_ int MPI_Dist_graph_neighbors(MPI_Comm comm, int maxindegree, int sources[], int sourceweights[], int maxoutdegree, int destinations[], int destweights[]) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Dist_graph_neighbors(comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Dist_graph_neighbors =============== */
static void MPI_Dist_graph_neighbors_fortran_wrapper(MPI_Fint *comm, MPI_Fint *maxindegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *maxoutdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Dist_graph_neighbors((MPI_Comm)(*comm), *maxindegree, (int*)sources, (int*)sourceweights, *maxoutdegree, (int*)destinations, (int*)destweights);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Dist_graph_neighbors(MPI_Comm_f2c(*comm), *maxindegree, (int*)sources, (int*)sourceweights, *maxoutdegree, (int*)destinations, (int*)destweights);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_DIST_GRAPH_NEIGHBORS(MPI_Fint *comm, MPI_Fint *maxindegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *maxoutdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_fortran_wrapper(comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights, ierr);
}

_EXTERN_C_ void mpi_dist_graph_neighbors(MPI_Fint *comm, MPI_Fint *maxindegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *maxoutdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_fortran_wrapper(comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights, ierr);
}

_EXTERN_C_ void mpi_dist_graph_neighbors_(MPI_Fint *comm, MPI_Fint *maxindegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *maxoutdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_fortran_wrapper(comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights, ierr);
}

_EXTERN_C_ void mpi_dist_graph_neighbors__(MPI_Fint *comm, MPI_Fint *maxindegree, MPI_Fint sources[], MPI_Fint sourceweights[], MPI_Fint *maxoutdegree, MPI_Fint destinations[], MPI_Fint destweights[], MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_fortran_wrapper(comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights, ierr);
}

/* ================= End Wrappers for MPI_Dist_graph_neighbors ================= */


/* ================== C Wrappers for MPI_Dist_graph_neighbors_count ================== */
_EXTERN_C_ int PMPI_Dist_graph_neighbors_count(MPI_Comm comm, int *inneighbors, int *outneighbors, int *weighted);
_EXTERN_C_ int MPI_Dist_graph_neighbors_count(MPI_Comm comm, int *inneighbors, int *outneighbors, int *weighted) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Dist_graph_neighbors_count(comm, inneighbors, outneighbors, weighted);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Dist_graph_neighbors_count =============== */
static void MPI_Dist_graph_neighbors_count_fortran_wrapper(MPI_Fint *comm, MPI_Fint *inneighbors, MPI_Fint *outneighbors, MPI_Fint *weighted, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Dist_graph_neighbors_count((MPI_Comm)(*comm), (int*)inneighbors, (int*)outneighbors, (int*)weighted);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Dist_graph_neighbors_count(MPI_Comm_f2c(*comm), (int*)inneighbors, (int*)outneighbors, (int*)weighted);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_DIST_GRAPH_NEIGHBORS_COUNT(MPI_Fint *comm, MPI_Fint *inneighbors, MPI_Fint *outneighbors, MPI_Fint *weighted, MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_count_fortran_wrapper(comm, inneighbors, outneighbors, weighted, ierr);
}

_EXTERN_C_ void mpi_dist_graph_neighbors_count(MPI_Fint *comm, MPI_Fint *inneighbors, MPI_Fint *outneighbors, MPI_Fint *weighted, MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_count_fortran_wrapper(comm, inneighbors, outneighbors, weighted, ierr);
}

_EXTERN_C_ void mpi_dist_graph_neighbors_count_(MPI_Fint *comm, MPI_Fint *inneighbors, MPI_Fint *outneighbors, MPI_Fint *weighted, MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_count_fortran_wrapper(comm, inneighbors, outneighbors, weighted, ierr);
}

_EXTERN_C_ void mpi_dist_graph_neighbors_count__(MPI_Fint *comm, MPI_Fint *inneighbors, MPI_Fint *outneighbors, MPI_Fint *weighted, MPI_Fint *ierr) { 
    MPI_Dist_graph_neighbors_count_fortran_wrapper(comm, inneighbors, outneighbors, weighted, ierr);
}

/* ================= End Wrappers for MPI_Dist_graph_neighbors_count ================= */


/* ================== C Wrappers for MPI_Errhandler_create ================== */
_EXTERN_C_ int PMPI_Errhandler_create(MPI_Handler_function *function, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_Errhandler_create(MPI_Handler_function *function, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Errhandler_create(function, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Errhandler_create =============== */
static void MPI_Errhandler_create_fortran_wrapper(MPI_Handler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Errhandler_create((MPI_Handler_function*)function, (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_Errhandler_create((MPI_Handler_function*)function, &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ERRHANDLER_CREATE(MPI_Handler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_create_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_create(MPI_Handler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_create_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_create_(MPI_Handler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_create_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_create__(MPI_Handler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_create_fortran_wrapper(function, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Errhandler_create ================= */


/* ================== C Wrappers for MPI_Errhandler_free ================== */
_EXTERN_C_ int PMPI_Errhandler_free(MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_Errhandler_free(MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Errhandler_free(errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Errhandler_free =============== */
static void MPI_Errhandler_free_fortran_wrapper(MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Errhandler_free((MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_Errhandler_free(&temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ERRHANDLER_FREE(MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_free_fortran_wrapper(errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_free(MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_free_fortran_wrapper(errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_free_(MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_free_fortran_wrapper(errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_free__(MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_free_fortran_wrapper(errhandler, ierr);
}

/* ================= End Wrappers for MPI_Errhandler_free ================= */


/* ================== C Wrappers for MPI_Errhandler_get ================== */
_EXTERN_C_ int PMPI_Errhandler_get(MPI_Comm comm, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_Errhandler_get(MPI_Comm comm, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Errhandler_get(comm, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Errhandler_get =============== */
static void MPI_Errhandler_get_fortran_wrapper(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Errhandler_get((MPI_Comm)(*comm), (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_Errhandler_get(MPI_Comm_f2c(*comm), &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ERRHANDLER_GET(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_get_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_get(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_get_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_get_(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_get_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_get__(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_get_fortran_wrapper(comm, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Errhandler_get ================= */


/* ================== C Wrappers for MPI_Errhandler_set ================== */
_EXTERN_C_ int PMPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
_EXTERN_C_ int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Errhandler_set(comm, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Errhandler_set =============== */
static void MPI_Errhandler_set_fortran_wrapper(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Errhandler_set((MPI_Comm)(*comm), (MPI_Errhandler)(*errhandler));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Errhandler_set(MPI_Comm_f2c(*comm), MPI_Errhandler_f2c(*errhandler));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ERRHANDLER_SET(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_set_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_set(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_set_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_set_(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_set_fortran_wrapper(comm, errhandler, ierr);
}

_EXTERN_C_ void mpi_errhandler_set__(MPI_Fint *comm, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Errhandler_set_fortran_wrapper(comm, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Errhandler_set ================= */


/* ================== C Wrappers for MPI_Error_class ================== */
_EXTERN_C_ int PMPI_Error_class(int errorcode, int *errorclass);
_EXTERN_C_ int MPI_Error_class(int errorcode, int *errorclass) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Error_class(errorcode, errorclass);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Error_class =============== */
static void MPI_Error_class_fortran_wrapper(MPI_Fint *errorcode, MPI_Fint *errorclass, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Error_class(*errorcode, (int*)errorclass);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ERROR_CLASS(MPI_Fint *errorcode, MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Error_class_fortran_wrapper(errorcode, errorclass, ierr);
}

_EXTERN_C_ void mpi_error_class(MPI_Fint *errorcode, MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Error_class_fortran_wrapper(errorcode, errorclass, ierr);
}

_EXTERN_C_ void mpi_error_class_(MPI_Fint *errorcode, MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Error_class_fortran_wrapper(errorcode, errorclass, ierr);
}

_EXTERN_C_ void mpi_error_class__(MPI_Fint *errorcode, MPI_Fint *errorclass, MPI_Fint *ierr) { 
    MPI_Error_class_fortran_wrapper(errorcode, errorclass, ierr);
}

/* ================= End Wrappers for MPI_Error_class ================= */


/* ================== C Wrappers for MPI_Error_string ================== */
_EXTERN_C_ int PMPI_Error_string(int errorcode, char *string, int *resultlen);
_EXTERN_C_ int MPI_Error_string(int errorcode, char *string, int *resultlen) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Error_string(errorcode, string, resultlen);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Error_string =============== */
static void MPI_Error_string_fortran_wrapper(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Error_string(*errorcode, (char*)string, (int*)resultlen);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ERROR_STRING(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Error_string_fortran_wrapper(errorcode, string, resultlen, ierr);
}

_EXTERN_C_ void mpi_error_string(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Error_string_fortran_wrapper(errorcode, string, resultlen, ierr);
}

_EXTERN_C_ void mpi_error_string_(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Error_string_fortran_wrapper(errorcode, string, resultlen, ierr);
}

_EXTERN_C_ void mpi_error_string__(MPI_Fint *errorcode, MPI_Fint *string, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Error_string_fortran_wrapper(errorcode, string, resultlen, ierr);
}

/* ================= End Wrappers for MPI_Error_string ================= */


/* ================== C Wrappers for MPI_Exscan ================== */
_EXTERN_C_ int PMPI_Exscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
_EXTERN_C_ int MPI_Exscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Exscan(sendbuf, recvbuf, count, datatype, op, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Exscan =============== */
static void MPI_Exscan_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Exscan((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Exscan((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_EXSCAN(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Exscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_exscan(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Exscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_exscan_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Exscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_exscan__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Exscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

/* ================= End Wrappers for MPI_Exscan ================= */


/* ================== C Wrappers for MPI_Fetch_and_op ================== */
_EXTERN_C_ int PMPI_Fetch_and_op(const void *origin_addr, void *result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Op op, MPI_Win win);
_EXTERN_C_ int MPI_Fetch_and_op(const void *origin_addr, void *result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Op op, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Fetch_and_op(origin_addr, result_addr, datatype, target_rank, target_disp, op, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Fetch_and_op =============== */
static void MPI_Fetch_and_op_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Fetch_and_op((const void*)origin_addr, (void*)result_addr, (MPI_Datatype)(*datatype), *target_rank, *target_disp, (MPI_Op)(*op), (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Fetch_and_op((const void*)origin_addr, (void*)result_addr, MPI_Type_f2c(*datatype), *target_rank, *target_disp, MPI_Op_f2c(*op), MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FETCH_AND_OP(MPI_Fint *origin_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Fetch_and_op_fortran_wrapper(origin_addr, result_addr, datatype, target_rank, target_disp, op, win, ierr);
}

_EXTERN_C_ void mpi_fetch_and_op(MPI_Fint *origin_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Fetch_and_op_fortran_wrapper(origin_addr, result_addr, datatype, target_rank, target_disp, op, win, ierr);
}

_EXTERN_C_ void mpi_fetch_and_op_(MPI_Fint *origin_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Fetch_and_op_fortran_wrapper(origin_addr, result_addr, datatype, target_rank, target_disp, op, win, ierr);
}

_EXTERN_C_ void mpi_fetch_and_op__(MPI_Fint *origin_addr, MPI_Fint *result_addr, MPI_Fint *datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Fetch_and_op_fortran_wrapper(origin_addr, result_addr, datatype, target_rank, target_disp, op, win, ierr);
}

/* ================= End Wrappers for MPI_Fetch_and_op ================= */


/* ================== C Wrappers for MPI_File_call_errhandler ================== */
_EXTERN_C_ int PMPI_File_call_errhandler(MPI_File fh, int errorcode);
_EXTERN_C_ int MPI_File_call_errhandler(MPI_File fh, int errorcode) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_call_errhandler(fh, errorcode);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_call_errhandler =============== */
static void MPI_File_call_errhandler_fortran_wrapper(MPI_Fint *fh, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_call_errhandler((MPI_File)(*fh), *errorcode);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_call_errhandler(MPI_File_f2c(*fh), *errorcode);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_CALL_ERRHANDLER(MPI_Fint *fh, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_File_call_errhandler_fortran_wrapper(fh, errorcode, ierr);
}

_EXTERN_C_ void mpi_file_call_errhandler(MPI_Fint *fh, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_File_call_errhandler_fortran_wrapper(fh, errorcode, ierr);
}

_EXTERN_C_ void mpi_file_call_errhandler_(MPI_Fint *fh, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_File_call_errhandler_fortran_wrapper(fh, errorcode, ierr);
}

_EXTERN_C_ void mpi_file_call_errhandler__(MPI_Fint *fh, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_File_call_errhandler_fortran_wrapper(fh, errorcode, ierr);
}

/* ================= End Wrappers for MPI_File_call_errhandler ================= */


/* ================== C Wrappers for MPI_File_close ================== */
_EXTERN_C_ int PMPI_File_close(MPI_File *fh);
_EXTERN_C_ int MPI_File_close(MPI_File *fh) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_close(fh);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_close =============== */
static void MPI_File_close_fortran_wrapper(MPI_Fint *fh, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_close((MPI_File*)fh);
#else /* MPI-2 safe call */
    MPI_File temp_fh;
    temp_fh = MPI_File_f2c(*fh);
    _wrap_py_return_val = MPI_File_close(&temp_fh);
    *fh = MPI_File_c2f(temp_fh);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_CLOSE(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_close_fortran_wrapper(fh, ierr);
}

_EXTERN_C_ void mpi_file_close(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_close_fortran_wrapper(fh, ierr);
}

_EXTERN_C_ void mpi_file_close_(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_close_fortran_wrapper(fh, ierr);
}

_EXTERN_C_ void mpi_file_close__(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_close_fortran_wrapper(fh, ierr);
}

/* ================= End Wrappers for MPI_File_close ================= */


/* ================== C Wrappers for MPI_File_create_errhandler ================== */
_EXTERN_C_ int PMPI_File_create_errhandler(MPI_File_errhandler_function *function, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_File_create_errhandler(MPI_File_errhandler_function *function, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_create_errhandler(function, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_create_errhandler =============== */
static void MPI_File_create_errhandler_fortran_wrapper(MPI_File_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_create_errhandler((MPI_File_errhandler_function*)function, (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_File_create_errhandler((MPI_File_errhandler_function*)function, &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_CREATE_ERRHANDLER(MPI_File_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_create_errhandler(MPI_File_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_create_errhandler_(MPI_File_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_create_errhandler__(MPI_File_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

/* ================= End Wrappers for MPI_File_create_errhandler ================= */


/* ================== C Wrappers for MPI_File_delete ================== */
_EXTERN_C_ int PMPI_File_delete(const char *filename, MPI_Info info);
_EXTERN_C_ int MPI_File_delete(const char *filename, MPI_Info info) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_delete(filename, info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_delete =============== */
static void MPI_File_delete_fortran_wrapper(MPI_Fint *filename, MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_delete((const char*)filename, (MPI_Info)(*info));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_delete((const char*)filename, MPI_Info_f2c(*info));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_DELETE(MPI_Fint *filename, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_delete_fortran_wrapper(filename, info, ierr);
}

_EXTERN_C_ void mpi_file_delete(MPI_Fint *filename, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_delete_fortran_wrapper(filename, info, ierr);
}

_EXTERN_C_ void mpi_file_delete_(MPI_Fint *filename, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_delete_fortran_wrapper(filename, info, ierr);
}

_EXTERN_C_ void mpi_file_delete__(MPI_Fint *filename, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_delete_fortran_wrapper(filename, info, ierr);
}

/* ================= End Wrappers for MPI_File_delete ================= */


/* ================== C Wrappers for MPI_File_get_amode ================== */
_EXTERN_C_ int PMPI_File_get_amode(MPI_File fh, int *amode);
_EXTERN_C_ int MPI_File_get_amode(MPI_File fh, int *amode) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_amode(fh, amode);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_amode =============== */
static void MPI_File_get_amode_fortran_wrapper(MPI_Fint *fh, MPI_Fint *amode, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_amode((MPI_File)(*fh), (int*)amode);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_amode(MPI_File_f2c(*fh), (int*)amode);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_AMODE(MPI_Fint *fh, MPI_Fint *amode, MPI_Fint *ierr) { 
    MPI_File_get_amode_fortran_wrapper(fh, amode, ierr);
}

_EXTERN_C_ void mpi_file_get_amode(MPI_Fint *fh, MPI_Fint *amode, MPI_Fint *ierr) { 
    MPI_File_get_amode_fortran_wrapper(fh, amode, ierr);
}

_EXTERN_C_ void mpi_file_get_amode_(MPI_Fint *fh, MPI_Fint *amode, MPI_Fint *ierr) { 
    MPI_File_get_amode_fortran_wrapper(fh, amode, ierr);
}

_EXTERN_C_ void mpi_file_get_amode__(MPI_Fint *fh, MPI_Fint *amode, MPI_Fint *ierr) { 
    MPI_File_get_amode_fortran_wrapper(fh, amode, ierr);
}

/* ================= End Wrappers for MPI_File_get_amode ================= */


/* ================== C Wrappers for MPI_File_get_atomicity ================== */
_EXTERN_C_ int PMPI_File_get_atomicity(MPI_File fh, int *flag);
_EXTERN_C_ int MPI_File_get_atomicity(MPI_File fh, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_atomicity(fh, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_atomicity =============== */
static void MPI_File_get_atomicity_fortran_wrapper(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_atomicity((MPI_File)(*fh), (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_atomicity(MPI_File_f2c(*fh), (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_ATOMICITY(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_get_atomicity_fortran_wrapper(fh, flag, ierr);
}

_EXTERN_C_ void mpi_file_get_atomicity(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_get_atomicity_fortran_wrapper(fh, flag, ierr);
}

_EXTERN_C_ void mpi_file_get_atomicity_(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_get_atomicity_fortran_wrapper(fh, flag, ierr);
}

_EXTERN_C_ void mpi_file_get_atomicity__(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_get_atomicity_fortran_wrapper(fh, flag, ierr);
}

/* ================= End Wrappers for MPI_File_get_atomicity ================= */


/* ================== C Wrappers for MPI_File_get_byte_offset ================== */
_EXTERN_C_ int PMPI_File_get_byte_offset(MPI_File fh, MPI_Offset offset, MPI_Offset *disp);
_EXTERN_C_ int MPI_File_get_byte_offset(MPI_File fh, MPI_Offset offset, MPI_Offset *disp) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_byte_offset(fh, offset, disp);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_byte_offset =============== */
static void MPI_File_get_byte_offset_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *disp, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_byte_offset((MPI_File)(*fh), *offset, (MPI_Offset*)disp);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_byte_offset(MPI_File_f2c(*fh), *offset, (MPI_Offset*)disp);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_BYTE_OFFSET(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *disp, MPI_Fint *ierr) { 
    MPI_File_get_byte_offset_fortran_wrapper(fh, offset, disp, ierr);
}

_EXTERN_C_ void mpi_file_get_byte_offset(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *disp, MPI_Fint *ierr) { 
    MPI_File_get_byte_offset_fortran_wrapper(fh, offset, disp, ierr);
}

_EXTERN_C_ void mpi_file_get_byte_offset_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *disp, MPI_Fint *ierr) { 
    MPI_File_get_byte_offset_fortran_wrapper(fh, offset, disp, ierr);
}

_EXTERN_C_ void mpi_file_get_byte_offset__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *disp, MPI_Fint *ierr) { 
    MPI_File_get_byte_offset_fortran_wrapper(fh, offset, disp, ierr);
}

/* ================= End Wrappers for MPI_File_get_byte_offset ================= */


/* ================== C Wrappers for MPI_File_get_errhandler ================== */
_EXTERN_C_ int PMPI_File_get_errhandler(MPI_File file, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_File_get_errhandler(MPI_File file, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_errhandler(file, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_errhandler =============== */
static void MPI_File_get_errhandler_fortran_wrapper(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_errhandler((MPI_File)(*file), (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_File_get_errhandler(MPI_File_f2c(*file), &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_ERRHANDLER(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_get_errhandler_fortran_wrapper(file, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_get_errhandler(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_get_errhandler_fortran_wrapper(file, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_get_errhandler_(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_get_errhandler_fortran_wrapper(file, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_get_errhandler__(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_get_errhandler_fortran_wrapper(file, errhandler, ierr);
}

/* ================= End Wrappers for MPI_File_get_errhandler ================= */


/* ================== C Wrappers for MPI_File_get_group ================== */
_EXTERN_C_ int PMPI_File_get_group(MPI_File fh, MPI_Group *group);
_EXTERN_C_ int MPI_File_get_group(MPI_File fh, MPI_Group *group) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_group(fh, group);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_group =============== */
static void MPI_File_get_group_fortran_wrapper(MPI_Fint *fh, MPI_Fint *group, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_group((MPI_File)(*fh), (MPI_Group*)group);
#else /* MPI-2 safe call */
    MPI_Group temp_group;
    temp_group = MPI_Group_f2c(*group);
    _wrap_py_return_val = MPI_File_get_group(MPI_File_f2c(*fh), &temp_group);
    *group = MPI_Group_c2f(temp_group);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_GROUP(MPI_Fint *fh, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_File_get_group_fortran_wrapper(fh, group, ierr);
}

_EXTERN_C_ void mpi_file_get_group(MPI_Fint *fh, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_File_get_group_fortran_wrapper(fh, group, ierr);
}

_EXTERN_C_ void mpi_file_get_group_(MPI_Fint *fh, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_File_get_group_fortran_wrapper(fh, group, ierr);
}

_EXTERN_C_ void mpi_file_get_group__(MPI_Fint *fh, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_File_get_group_fortran_wrapper(fh, group, ierr);
}

/* ================= End Wrappers for MPI_File_get_group ================= */


/* ================== C Wrappers for MPI_File_get_info ================== */
_EXTERN_C_ int PMPI_File_get_info(MPI_File fh, MPI_Info *info_used);
_EXTERN_C_ int MPI_File_get_info(MPI_File fh, MPI_Info *info_used) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_info(fh, info_used);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_info =============== */
static void MPI_File_get_info_fortran_wrapper(MPI_Fint *fh, MPI_Fint *info_used, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_info((MPI_File)(*fh), (MPI_Info*)info_used);
#else /* MPI-2 safe call */
    MPI_Info temp_info_used;
    temp_info_used = MPI_Info_f2c(*info_used);
    _wrap_py_return_val = MPI_File_get_info(MPI_File_f2c(*fh), &temp_info_used);
    *info_used = MPI_Info_c2f(temp_info_used);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_INFO(MPI_Fint *fh, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_File_get_info_fortran_wrapper(fh, info_used, ierr);
}

_EXTERN_C_ void mpi_file_get_info(MPI_Fint *fh, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_File_get_info_fortran_wrapper(fh, info_used, ierr);
}

_EXTERN_C_ void mpi_file_get_info_(MPI_Fint *fh, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_File_get_info_fortran_wrapper(fh, info_used, ierr);
}

_EXTERN_C_ void mpi_file_get_info__(MPI_Fint *fh, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_File_get_info_fortran_wrapper(fh, info_used, ierr);
}

/* ================= End Wrappers for MPI_File_get_info ================= */


/* ================== C Wrappers for MPI_File_get_position ================== */
_EXTERN_C_ int PMPI_File_get_position(MPI_File fh, MPI_Offset *offset);
_EXTERN_C_ int MPI_File_get_position(MPI_File fh, MPI_Offset *offset) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_position(fh, offset);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_position =============== */
static void MPI_File_get_position_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_position((MPI_File)(*fh), (MPI_Offset*)offset);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_position(MPI_File_f2c(*fh), (MPI_Offset*)offset);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_POSITION(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_fortran_wrapper(fh, offset, ierr);
}

_EXTERN_C_ void mpi_file_get_position(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_fortran_wrapper(fh, offset, ierr);
}

_EXTERN_C_ void mpi_file_get_position_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_fortran_wrapper(fh, offset, ierr);
}

_EXTERN_C_ void mpi_file_get_position__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_fortran_wrapper(fh, offset, ierr);
}

/* ================= End Wrappers for MPI_File_get_position ================= */


/* ================== C Wrappers for MPI_File_get_position_shared ================== */
_EXTERN_C_ int PMPI_File_get_position_shared(MPI_File fh, MPI_Offset *offset);
_EXTERN_C_ int MPI_File_get_position_shared(MPI_File fh, MPI_Offset *offset) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_position_shared(fh, offset);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_position_shared =============== */
static void MPI_File_get_position_shared_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_position_shared((MPI_File)(*fh), (MPI_Offset*)offset);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_position_shared(MPI_File_f2c(*fh), (MPI_Offset*)offset);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_POSITION_SHARED(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_shared_fortran_wrapper(fh, offset, ierr);
}

_EXTERN_C_ void mpi_file_get_position_shared(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_shared_fortran_wrapper(fh, offset, ierr);
}

_EXTERN_C_ void mpi_file_get_position_shared_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_shared_fortran_wrapper(fh, offset, ierr);
}

_EXTERN_C_ void mpi_file_get_position_shared__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *ierr) { 
    MPI_File_get_position_shared_fortran_wrapper(fh, offset, ierr);
}

/* ================= End Wrappers for MPI_File_get_position_shared ================= */


/* ================== C Wrappers for MPI_File_get_size ================== */
_EXTERN_C_ int PMPI_File_get_size(MPI_File fh, MPI_Offset *size);
_EXTERN_C_ int MPI_File_get_size(MPI_File fh, MPI_Offset *size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_size(fh, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_size =============== */
static void MPI_File_get_size_fortran_wrapper(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_size((MPI_File)(*fh), (MPI_Offset*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_size(MPI_File_f2c(*fh), (MPI_Offset*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_SIZE(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_get_size_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_get_size(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_get_size_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_get_size_(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_get_size_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_get_size__(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_get_size_fortran_wrapper(fh, size, ierr);
}

/* ================= End Wrappers for MPI_File_get_size ================= */


/* ================== C Wrappers for MPI_File_get_type_extent ================== */
_EXTERN_C_ int PMPI_File_get_type_extent(MPI_File fh, MPI_Datatype datatype, MPI_Aint *extent);
_EXTERN_C_ int MPI_File_get_type_extent(MPI_File fh, MPI_Datatype datatype, MPI_Aint *extent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_type_extent(fh, datatype, extent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_type_extent =============== */
static void MPI_File_get_type_extent_fortran_wrapper(MPI_Fint *fh, MPI_Fint *datatype, MPI_Aint *extent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_type_extent((MPI_File)(*fh), (MPI_Datatype)(*datatype), (MPI_Aint*)extent);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_get_type_extent(MPI_File_f2c(*fh), MPI_Type_f2c(*datatype), (MPI_Aint*)extent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_TYPE_EXTENT(MPI_Fint *fh, MPI_Fint *datatype, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_File_get_type_extent_fortran_wrapper(fh, datatype, extent, ierr);
}

_EXTERN_C_ void mpi_file_get_type_extent(MPI_Fint *fh, MPI_Fint *datatype, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_File_get_type_extent_fortran_wrapper(fh, datatype, extent, ierr);
}

_EXTERN_C_ void mpi_file_get_type_extent_(MPI_Fint *fh, MPI_Fint *datatype, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_File_get_type_extent_fortran_wrapper(fh, datatype, extent, ierr);
}

_EXTERN_C_ void mpi_file_get_type_extent__(MPI_Fint *fh, MPI_Fint *datatype, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_File_get_type_extent_fortran_wrapper(fh, datatype, extent, ierr);
}

/* ================= End Wrappers for MPI_File_get_type_extent ================= */


/* ================== C Wrappers for MPI_File_get_view ================== */
_EXTERN_C_ int PMPI_File_get_view(MPI_File fh, MPI_Offset *disp, MPI_Datatype *etype, MPI_Datatype *filetype, char *datarep);
_EXTERN_C_ int MPI_File_get_view(MPI_File fh, MPI_Offset *disp, MPI_Datatype *etype, MPI_Datatype *filetype, char *datarep) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_get_view(fh, disp, etype, filetype, datarep);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_get_view =============== */
static void MPI_File_get_view_fortran_wrapper(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_get_view((MPI_File)(*fh), (MPI_Offset*)disp, (MPI_Datatype*)etype, (MPI_Datatype*)filetype, (char*)datarep);
#else /* MPI-2 safe call */
    MPI_Datatype temp_filetype;
    MPI_Datatype temp_etype;
    temp_etype = MPI_Type_f2c(*etype);
    temp_filetype = MPI_Type_f2c(*filetype);
    _wrap_py_return_val = MPI_File_get_view(MPI_File_f2c(*fh), (MPI_Offset*)disp, &temp_etype, &temp_filetype, (char*)datarep);
    *etype = MPI_Type_c2f(temp_etype);
    *filetype = MPI_Type_c2f(temp_filetype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_GET_VIEW(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *ierr) { 
    MPI_File_get_view_fortran_wrapper(fh, disp, etype, filetype, datarep, ierr);
}

_EXTERN_C_ void mpi_file_get_view(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *ierr) { 
    MPI_File_get_view_fortran_wrapper(fh, disp, etype, filetype, datarep, ierr);
}

_EXTERN_C_ void mpi_file_get_view_(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *ierr) { 
    MPI_File_get_view_fortran_wrapper(fh, disp, etype, filetype, datarep, ierr);
}

_EXTERN_C_ void mpi_file_get_view__(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *ierr) { 
    MPI_File_get_view_fortran_wrapper(fh, disp, etype, filetype, datarep, ierr);
}

/* ================= End Wrappers for MPI_File_get_view ================= */


/* ================== C Wrappers for MPI_File_iread ================== */
_EXTERN_C_ int PMPI_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iread(fh, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iread =============== */
static void MPI_File_iread_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iread((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iread(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IREAD(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iread ================= */


/* ================== C Wrappers for MPI_File_iread_all ================== */
_EXTERN_C_ int PMPI_File_iread_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iread_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iread_all(fh, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iread_all =============== */
static void MPI_File_iread_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iread_all((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iread_all(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IREAD_ALL(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_all(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_all_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_all__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iread_all ================= */


/* ================== C Wrappers for MPI_File_iread_at ================== */
_EXTERN_C_ int PMPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iread_at(fh, offset, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iread_at =============== */
static void MPI_File_iread_at_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iread_at((MPI_File)(*fh), *offset, (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iread_at(MPI_File_f2c(*fh), *offset, (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IREAD_AT(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_at(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_at_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_at__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iread_at ================= */


/* ================== C Wrappers for MPI_File_iread_at_all ================== */
_EXTERN_C_ int PMPI_File_iread_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iread_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iread_at_all(fh, offset, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iread_at_all =============== */
static void MPI_File_iread_at_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iread_at_all((MPI_File)(*fh), *offset, (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iread_at_all(MPI_File_f2c(*fh), *offset, (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IREAD_AT_ALL(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_at_all(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_at_all_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_at_all__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iread_at_all ================= */


/* ================== C Wrappers for MPI_File_iread_shared ================== */
_EXTERN_C_ int PMPI_File_iread_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iread_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iread_shared(fh, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iread_shared =============== */
static void MPI_File_iread_shared_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iread_shared((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iread_shared(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IREAD_SHARED(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_shared(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_shared_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iread_shared__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iread_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iread_shared ================= */


/* ================== C Wrappers for MPI_File_iwrite ================== */
_EXTERN_C_ int PMPI_File_iwrite(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iwrite(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iwrite(fh, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iwrite =============== */
static void MPI_File_iwrite_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iwrite((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iwrite(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IWRITE(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iwrite ================= */


/* ================== C Wrappers for MPI_File_iwrite_all ================== */
_EXTERN_C_ int PMPI_File_iwrite_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iwrite_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iwrite_all(fh, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iwrite_all =============== */
static void MPI_File_iwrite_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iwrite_all((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iwrite_all(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IWRITE_ALL(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_all(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_all_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_all__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_all_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iwrite_all ================= */


/* ================== C Wrappers for MPI_File_iwrite_at ================== */
_EXTERN_C_ int PMPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iwrite_at(fh, offset, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iwrite_at =============== */
static void MPI_File_iwrite_at_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iwrite_at((MPI_File)(*fh), *offset, (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iwrite_at(MPI_File_f2c(*fh), *offset, (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IWRITE_AT(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_at(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_at_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_at__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iwrite_at ================= */


/* ================== C Wrappers for MPI_File_iwrite_at_all ================== */
_EXTERN_C_ int PMPI_File_iwrite_at_all(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iwrite_at_all(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iwrite_at_all(fh, offset, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iwrite_at_all =============== */
static void MPI_File_iwrite_at_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iwrite_at_all((MPI_File)(*fh), *offset, (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iwrite_at_all(MPI_File_f2c(*fh), *offset, (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IWRITE_AT_ALL(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_at_all(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_at_all_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_at_all__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_at_all_fortran_wrapper(fh, offset, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iwrite_at_all ================= */


/* ================== C Wrappers for MPI_File_iwrite_shared ================== */
_EXTERN_C_ int PMPI_File_iwrite_shared(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
_EXTERN_C_ int MPI_File_iwrite_shared(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_iwrite_shared(fh, buf, count, datatype, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_iwrite_shared =============== */
static void MPI_File_iwrite_shared_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_iwrite_shared((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_File_iwrite_shared(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_IWRITE_SHARED(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_shared(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_shared_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

_EXTERN_C_ void mpi_file_iwrite_shared__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_File_iwrite_shared_fortran_wrapper(fh, buf, count, datatype, request, ierr);
}

/* ================= End Wrappers for MPI_File_iwrite_shared ================= */


/* ================== C Wrappers for MPI_File_open ================== */
_EXTERN_C_ int PMPI_File_open(MPI_Comm comm, const char *filename, int amode, MPI_Info info, MPI_File *fh);
_EXTERN_C_ int MPI_File_open(MPI_Comm comm, const char *filename, int amode, MPI_Info info, MPI_File *fh) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_File_open(comm, filename, amode, info, fh);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_open =============== */
static void MPI_File_open_fortran_wrapper(MPI_Fint *comm, MPI_Fint *filename, MPI_Fint *amode, MPI_Fint *info, MPI_Fint *fh, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_open((MPI_Comm)(*comm), (const char*)filename, *amode, (MPI_Info)(*info), (MPI_File*)fh);
#else /* MPI-2 safe call */
    MPI_File temp_fh;
    temp_fh = MPI_File_f2c(*fh);
    _wrap_py_return_val = MPI_File_open(MPI_Comm_f2c(*comm), (const char*)filename, *amode, MPI_Info_f2c(*info), &temp_fh);
    *fh = MPI_File_c2f(temp_fh);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_OPEN(MPI_Fint *comm, MPI_Fint *filename, MPI_Fint *amode, MPI_Fint *info, MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_open_fortran_wrapper(comm, filename, amode, info, fh, ierr);
}

_EXTERN_C_ void mpi_file_open(MPI_Fint *comm, MPI_Fint *filename, MPI_Fint *amode, MPI_Fint *info, MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_open_fortran_wrapper(comm, filename, amode, info, fh, ierr);
}

_EXTERN_C_ void mpi_file_open_(MPI_Fint *comm, MPI_Fint *filename, MPI_Fint *amode, MPI_Fint *info, MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_open_fortran_wrapper(comm, filename, amode, info, fh, ierr);
}

_EXTERN_C_ void mpi_file_open__(MPI_Fint *comm, MPI_Fint *filename, MPI_Fint *amode, MPI_Fint *info, MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_open_fortran_wrapper(comm, filename, amode, info, fh, ierr);
}

/* ================= End Wrappers for MPI_File_open ================= */


/* ================== C Wrappers for MPI_File_preallocate ================== */
_EXTERN_C_ int PMPI_File_preallocate(MPI_File fh, MPI_Offset size);
_EXTERN_C_ int MPI_File_preallocate(MPI_File fh, MPI_Offset size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_preallocate(fh, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_preallocate =============== */
static void MPI_File_preallocate_fortran_wrapper(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_preallocate((MPI_File)(*fh), *size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_preallocate(MPI_File_f2c(*fh), *size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_PREALLOCATE(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_preallocate_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_preallocate(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_preallocate_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_preallocate_(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_preallocate_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_preallocate__(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_preallocate_fortran_wrapper(fh, size, ierr);
}

/* ================= End Wrappers for MPI_File_preallocate ================= */


/* ================== C Wrappers for MPI_File_read ================== */
_EXTERN_C_ int PMPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read =============== */
static void MPI_File_read_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_read ================= */


/* ================== C Wrappers for MPI_File_read_all ================== */
_EXTERN_C_ int PMPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_all(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_all =============== */
static void MPI_File_read_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_all((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_all(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_ALL(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_all(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_all_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_all__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_all ================= */


/* ================== C Wrappers for MPI_File_read_all_begin ================== */
_EXTERN_C_ int PMPI_File_read_all_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype);
_EXTERN_C_ int MPI_File_read_all_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_all_begin(fh, buf, count, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_all_begin =============== */
static void MPI_File_read_all_begin_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_all_begin((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_read_all_begin(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_ALL_BEGIN(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_all_begin(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_all_begin_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_all_begin__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

/* ================= End Wrappers for MPI_File_read_all_begin ================= */


/* ================== C Wrappers for MPI_File_read_all_end ================== */
_EXTERN_C_ int PMPI_File_read_all_end(MPI_File fh, void *buf, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_all_end(MPI_File fh, void *buf, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_all_end(fh, buf, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_all_end =============== */
static void MPI_File_read_all_end_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_all_end((MPI_File)(*fh), (void*)buf, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_all_end(MPI_File_f2c(*fh), (void*)buf, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_ALL_END(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_all_end(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_all_end_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_all_end__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_all_end_fortran_wrapper(fh, buf, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_all_end ================= */


/* ================== C Wrappers for MPI_File_read_at ================== */
_EXTERN_C_ int PMPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_at(fh, offset, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_at =============== */
static void MPI_File_read_at_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_at((MPI_File)(*fh), *offset, (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_at(MPI_File_f2c(*fh), *offset, (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_AT(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_at ================= */


/* ================== C Wrappers for MPI_File_read_at_all ================== */
_EXTERN_C_ int PMPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_at_all(fh, offset, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_at_all =============== */
static void MPI_File_read_at_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_at_all((MPI_File)(*fh), *offset, (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_at_all(MPI_File_f2c(*fh), *offset, (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_AT_ALL(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_at_all ================= */


/* ================== C Wrappers for MPI_File_read_at_all_begin ================== */
_EXTERN_C_ int PMPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype);
_EXTERN_C_ int MPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_at_all_begin(fh, offset, buf, count, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_at_all_begin =============== */
static void MPI_File_read_at_all_begin_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_at_all_begin((MPI_File)(*fh), *offset, (void*)buf, *count, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_read_at_all_begin(MPI_File_f2c(*fh), *offset, (void*)buf, *count, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_AT_ALL_BEGIN(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_begin(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_begin_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_begin__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

/* ================= End Wrappers for MPI_File_read_at_all_begin ================= */


/* ================== C Wrappers for MPI_File_read_at_all_end ================== */
_EXTERN_C_ int PMPI_File_read_at_all_end(MPI_File fh, void *buf, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_at_all_end(MPI_File fh, void *buf, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_at_all_end(fh, buf, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_at_all_end =============== */
static void MPI_File_read_at_all_end_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_at_all_end((MPI_File)(*fh), (void*)buf, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_at_all_end(MPI_File_f2c(*fh), (void*)buf, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_AT_ALL_END(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_end(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_end_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_at_all_end__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_at_all_end ================= */


/* ================== C Wrappers for MPI_File_read_ordered ================== */
_EXTERN_C_ int PMPI_File_read_ordered(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_ordered(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_ordered(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_ordered =============== */
static void MPI_File_read_ordered_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_ordered((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_ordered(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_ORDERED(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_ordered ================= */


/* ================== C Wrappers for MPI_File_read_ordered_begin ================== */
_EXTERN_C_ int PMPI_File_read_ordered_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype);
_EXTERN_C_ int MPI_File_read_ordered_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_ordered_begin(fh, buf, count, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_ordered_begin =============== */
static void MPI_File_read_ordered_begin_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_ordered_begin((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_read_ordered_begin(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_ORDERED_BEGIN(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_begin(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_begin_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_begin__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_read_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

/* ================= End Wrappers for MPI_File_read_ordered_begin ================= */


/* ================== C Wrappers for MPI_File_read_ordered_end ================== */
_EXTERN_C_ int PMPI_File_read_ordered_end(MPI_File fh, void *buf, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_ordered_end(MPI_File fh, void *buf, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_ordered_end(fh, buf, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_ordered_end =============== */
static void MPI_File_read_ordered_end_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_ordered_end((MPI_File)(*fh), (void*)buf, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_ordered_end(MPI_File_f2c(*fh), (void*)buf, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_ORDERED_END(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_end(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_end_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_read_ordered_end__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_ordered_end ================= */


/* ================== C Wrappers for MPI_File_read_shared ================== */
_EXTERN_C_ int PMPI_File_read_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_read_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_read_shared(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_read_shared =============== */
static void MPI_File_read_shared_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_read_shared((MPI_File)(*fh), (void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_read_shared(MPI_File_f2c(*fh), (void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_READ_SHARED(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_shared(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_shared_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_read_shared__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_read_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_read_shared ================= */


/* ================== C Wrappers for MPI_File_seek ================== */
_EXTERN_C_ int PMPI_File_seek(MPI_File fh, MPI_Offset offset, int whence);
_EXTERN_C_ int MPI_File_seek(MPI_File fh, MPI_Offset offset, int whence) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_seek(fh, offset, whence);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_seek =============== */
static void MPI_File_seek_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_seek((MPI_File)(*fh), *offset, *whence);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_seek(MPI_File_f2c(*fh), *offset, *whence);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SEEK(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_fortran_wrapper(fh, offset, whence, ierr);
}

_EXTERN_C_ void mpi_file_seek(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_fortran_wrapper(fh, offset, whence, ierr);
}

_EXTERN_C_ void mpi_file_seek_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_fortran_wrapper(fh, offset, whence, ierr);
}

_EXTERN_C_ void mpi_file_seek__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_fortran_wrapper(fh, offset, whence, ierr);
}

/* ================= End Wrappers for MPI_File_seek ================= */


/* ================== C Wrappers for MPI_File_seek_shared ================== */
_EXTERN_C_ int PMPI_File_seek_shared(MPI_File fh, MPI_Offset offset, int whence);
_EXTERN_C_ int MPI_File_seek_shared(MPI_File fh, MPI_Offset offset, int whence) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_seek_shared(fh, offset, whence);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_seek_shared =============== */
static void MPI_File_seek_shared_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_seek_shared((MPI_File)(*fh), *offset, *whence);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_seek_shared(MPI_File_f2c(*fh), *offset, *whence);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SEEK_SHARED(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_shared_fortran_wrapper(fh, offset, whence, ierr);
}

_EXTERN_C_ void mpi_file_seek_shared(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_shared_fortran_wrapper(fh, offset, whence, ierr);
}

_EXTERN_C_ void mpi_file_seek_shared_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_shared_fortran_wrapper(fh, offset, whence, ierr);
}

_EXTERN_C_ void mpi_file_seek_shared__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *whence, MPI_Fint *ierr) { 
    MPI_File_seek_shared_fortran_wrapper(fh, offset, whence, ierr);
}

/* ================= End Wrappers for MPI_File_seek_shared ================= */


/* ================== C Wrappers for MPI_File_set_atomicity ================== */
_EXTERN_C_ int PMPI_File_set_atomicity(MPI_File fh, int flag);
_EXTERN_C_ int MPI_File_set_atomicity(MPI_File fh, int flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_set_atomicity(fh, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_set_atomicity =============== */
static void MPI_File_set_atomicity_fortran_wrapper(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_set_atomicity((MPI_File)(*fh), *flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_set_atomicity(MPI_File_f2c(*fh), *flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SET_ATOMICITY(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_set_atomicity_fortran_wrapper(fh, flag, ierr);
}

_EXTERN_C_ void mpi_file_set_atomicity(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_set_atomicity_fortran_wrapper(fh, flag, ierr);
}

_EXTERN_C_ void mpi_file_set_atomicity_(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_set_atomicity_fortran_wrapper(fh, flag, ierr);
}

_EXTERN_C_ void mpi_file_set_atomicity__(MPI_Fint *fh, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_File_set_atomicity_fortran_wrapper(fh, flag, ierr);
}

/* ================= End Wrappers for MPI_File_set_atomicity ================= */


/* ================== C Wrappers for MPI_File_set_errhandler ================== */
_EXTERN_C_ int PMPI_File_set_errhandler(MPI_File file, MPI_Errhandler errhandler);
_EXTERN_C_ int MPI_File_set_errhandler(MPI_File file, MPI_Errhandler errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_set_errhandler(file, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_set_errhandler =============== */
static void MPI_File_set_errhandler_fortran_wrapper(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_set_errhandler((MPI_File)(*file), (MPI_Errhandler)(*errhandler));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_set_errhandler(MPI_File_f2c(*file), MPI_Errhandler_f2c(*errhandler));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SET_ERRHANDLER(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_set_errhandler_fortran_wrapper(file, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_set_errhandler(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_set_errhandler_fortran_wrapper(file, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_set_errhandler_(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_set_errhandler_fortran_wrapper(file, errhandler, ierr);
}

_EXTERN_C_ void mpi_file_set_errhandler__(MPI_Fint *file, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_File_set_errhandler_fortran_wrapper(file, errhandler, ierr);
}

/* ================= End Wrappers for MPI_File_set_errhandler ================= */


/* ================== C Wrappers for MPI_File_set_info ================== */
_EXTERN_C_ int PMPI_File_set_info(MPI_File fh, MPI_Info info);
_EXTERN_C_ int MPI_File_set_info(MPI_File fh, MPI_Info info) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_set_info(fh, info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_set_info =============== */
static void MPI_File_set_info_fortran_wrapper(MPI_Fint *fh, MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_set_info((MPI_File)(*fh), (MPI_Info)(*info));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_set_info(MPI_File_f2c(*fh), MPI_Info_f2c(*info));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SET_INFO(MPI_Fint *fh, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_info_fortran_wrapper(fh, info, ierr);
}

_EXTERN_C_ void mpi_file_set_info(MPI_Fint *fh, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_info_fortran_wrapper(fh, info, ierr);
}

_EXTERN_C_ void mpi_file_set_info_(MPI_Fint *fh, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_info_fortran_wrapper(fh, info, ierr);
}

_EXTERN_C_ void mpi_file_set_info__(MPI_Fint *fh, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_info_fortran_wrapper(fh, info, ierr);
}

/* ================= End Wrappers for MPI_File_set_info ================= */


/* ================== C Wrappers for MPI_File_set_size ================== */
_EXTERN_C_ int PMPI_File_set_size(MPI_File fh, MPI_Offset size);
_EXTERN_C_ int MPI_File_set_size(MPI_File fh, MPI_Offset size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_set_size(fh, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_set_size =============== */
static void MPI_File_set_size_fortran_wrapper(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_set_size((MPI_File)(*fh), *size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_set_size(MPI_File_f2c(*fh), *size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SET_SIZE(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_set_size_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_set_size(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_set_size_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_set_size_(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_set_size_fortran_wrapper(fh, size, ierr);
}

_EXTERN_C_ void mpi_file_set_size__(MPI_Fint *fh, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_File_set_size_fortran_wrapper(fh, size, ierr);
}

/* ================= End Wrappers for MPI_File_set_size ================= */


/* ================== C Wrappers for MPI_File_set_view ================== */
_EXTERN_C_ int PMPI_File_set_view(MPI_File fh, MPI_Offset disp, MPI_Datatype etype, MPI_Datatype filetype, const char *datarep, MPI_Info info);
_EXTERN_C_ int MPI_File_set_view(MPI_File fh, MPI_Offset disp, MPI_Datatype etype, MPI_Datatype filetype, const char *datarep, MPI_Info info) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_set_view(fh, disp, etype, filetype, datarep, info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_set_view =============== */
static void MPI_File_set_view_fortran_wrapper(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_set_view((MPI_File)(*fh), *disp, (MPI_Datatype)(*etype), (MPI_Datatype)(*filetype), (const char*)datarep, (MPI_Info)(*info));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_set_view(MPI_File_f2c(*fh), *disp, MPI_Type_f2c(*etype), MPI_Type_f2c(*filetype), (const char*)datarep, MPI_Info_f2c(*info));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SET_VIEW(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_view_fortran_wrapper(fh, disp, etype, filetype, datarep, info, ierr);
}

_EXTERN_C_ void mpi_file_set_view(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_view_fortran_wrapper(fh, disp, etype, filetype, datarep, info, ierr);
}

_EXTERN_C_ void mpi_file_set_view_(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_view_fortran_wrapper(fh, disp, etype, filetype, datarep, info, ierr);
}

_EXTERN_C_ void mpi_file_set_view__(MPI_Fint *fh, MPI_Fint *disp, MPI_Fint *etype, MPI_Fint *filetype, MPI_Fint *datarep, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_File_set_view_fortran_wrapper(fh, disp, etype, filetype, datarep, info, ierr);
}

/* ================= End Wrappers for MPI_File_set_view ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int PMPI_File_sync(MPI_File fh);
_EXTERN_C_ int MPI_File_sync(MPI_File fh) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_sync(fh);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_File_sync_fortran_wrapper(MPI_Fint *fh, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_sync((MPI_File)(*fh));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_sync(MPI_File_f2c(*fh));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_SYNC(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_sync_fortran_wrapper(fh, ierr);
}

_EXTERN_C_ void mpi_file_sync(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_sync_fortran_wrapper(fh, ierr);
}

_EXTERN_C_ void mpi_file_sync_(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_sync_fortran_wrapper(fh, ierr);
}

_EXTERN_C_ void mpi_file_sync__(MPI_Fint *fh, MPI_Fint *ierr) { 
    MPI_File_sync_fortran_wrapper(fh, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_write ================== */
_EXTERN_C_ int PMPI_File_write(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_write(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write =============== */
static void MPI_File_write_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_write ================= */


/* ================== C Wrappers for MPI_File_write_all ================== */
_EXTERN_C_ int PMPI_File_write_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_all(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_all =============== */
static void MPI_File_write_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_all((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_all(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_ALL(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_all(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_all_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_all__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_all ================= */


/* ================== C Wrappers for MPI_File_write_all_begin ================== */
_EXTERN_C_ int PMPI_File_write_all_begin(MPI_File fh, const void *buf, int count, MPI_Datatype datatype);
_EXTERN_C_ int MPI_File_write_all_begin(MPI_File fh, const void *buf, int count, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_all_begin(fh, buf, count, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_all_begin =============== */
static void MPI_File_write_all_begin_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_all_begin((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_write_all_begin(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_ALL_BEGIN(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_all_begin(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_all_begin_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_all_begin__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_all_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

/* ================= End Wrappers for MPI_File_write_all_begin ================= */


/* ================== C Wrappers for MPI_File_write_all_end ================== */
_EXTERN_C_ int PMPI_File_write_all_end(MPI_File fh, const void *buf, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_all_end(MPI_File fh, const void *buf, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_all_end(fh, buf, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_all_end =============== */
static void MPI_File_write_all_end_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_all_end((MPI_File)(*fh), (const void*)buf, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_all_end(MPI_File_f2c(*fh), (const void*)buf, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_ALL_END(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_all_end(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_all_end_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_all_end__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_all_end_fortran_wrapper(fh, buf, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_all_end ================= */


/* ================== C Wrappers for MPI_File_write_at ================== */
_EXTERN_C_ int PMPI_File_write_at(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_at(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_at(fh, offset, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_at =============== */
static void MPI_File_write_at_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_at((MPI_File)(*fh), *offset, (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_at(MPI_File_f2c(*fh), *offset, (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_AT(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_at ================= */


/* ================== C Wrappers for MPI_File_write_at_all ================== */
_EXTERN_C_ int PMPI_File_write_at_all(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_at_all(fh, offset, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_at_all =============== */
static void MPI_File_write_at_all_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_at_all((MPI_File)(*fh), *offset, (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_at_all(MPI_File_f2c(*fh), *offset, (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_AT_ALL(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_fortran_wrapper(fh, offset, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_at_all ================= */


/* ================== C Wrappers for MPI_File_write_at_all_begin ================== */
_EXTERN_C_ int PMPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype);
_EXTERN_C_ int MPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_at_all_begin(fh, offset, buf, count, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_at_all_begin =============== */
static void MPI_File_write_at_all_begin_fortran_wrapper(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_at_all_begin((MPI_File)(*fh), *offset, (const void*)buf, *count, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_write_at_all_begin(MPI_File_f2c(*fh), *offset, (const void*)buf, *count, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_AT_ALL_BEGIN(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_begin(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_begin_(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_begin__(MPI_Fint *fh, MPI_Fint *offset, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_at_all_begin_fortran_wrapper(fh, offset, buf, count, datatype, ierr);
}

/* ================= End Wrappers for MPI_File_write_at_all_begin ================= */


/* ================== C Wrappers for MPI_File_write_at_all_end ================== */
_EXTERN_C_ int PMPI_File_write_at_all_end(MPI_File fh, const void *buf, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_at_all_end(MPI_File fh, const void *buf, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_at_all_end(fh, buf, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_at_all_end =============== */
static void MPI_File_write_at_all_end_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_at_all_end((MPI_File)(*fh), (const void*)buf, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_at_all_end(MPI_File_f2c(*fh), (const void*)buf, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_AT_ALL_END(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_end(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_end_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_at_all_end__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_at_all_end_fortran_wrapper(fh, buf, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_at_all_end ================= */


/* ================== C Wrappers for MPI_File_write_ordered ================== */
_EXTERN_C_ int PMPI_File_write_ordered(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_ordered(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_ordered(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_ordered =============== */
static void MPI_File_write_ordered_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_ordered((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_ordered(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_ORDERED(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_ordered ================= */


/* ================== C Wrappers for MPI_File_write_ordered_begin ================== */
_EXTERN_C_ int PMPI_File_write_ordered_begin(MPI_File fh, const void *buf, int count, MPI_Datatype datatype);
_EXTERN_C_ int MPI_File_write_ordered_begin(MPI_File fh, const void *buf, int count, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_ordered_begin(fh, buf, count, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_ordered_begin =============== */
static void MPI_File_write_ordered_begin_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_ordered_begin((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_File_write_ordered_begin(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_ORDERED_BEGIN(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_begin(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_begin_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_begin__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_File_write_ordered_begin_fortran_wrapper(fh, buf, count, datatype, ierr);
}

/* ================= End Wrappers for MPI_File_write_ordered_begin ================= */


/* ================== C Wrappers for MPI_File_write_ordered_end ================== */
_EXTERN_C_ int PMPI_File_write_ordered_end(MPI_File fh, const void *buf, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_ordered_end(MPI_File fh, const void *buf, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_ordered_end(fh, buf, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_ordered_end =============== */
static void MPI_File_write_ordered_end_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_ordered_end((MPI_File)(*fh), (const void*)buf, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_ordered_end(MPI_File_f2c(*fh), (const void*)buf, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_ORDERED_END(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_end(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_end_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

_EXTERN_C_ void mpi_file_write_ordered_end__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_ordered_end_fortran_wrapper(fh, buf, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_ordered_end ================= */


/* ================== C Wrappers for MPI_File_write_shared ================== */
_EXTERN_C_ int PMPI_File_write_shared(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
_EXTERN_C_ int MPI_File_write_shared(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_File_write_shared(fh, buf, count, datatype, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_File_write_shared =============== */
static void MPI_File_write_shared_fortran_wrapper(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_File_write_shared((MPI_File)(*fh), (const void*)buf, *count, (MPI_Datatype)(*datatype), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_File_write_shared(MPI_File_f2c(*fh), (const void*)buf, *count, MPI_Type_f2c(*datatype), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FILE_WRITE_SHARED(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_shared(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_shared_(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

_EXTERN_C_ void mpi_file_write_shared__(MPI_Fint *fh, MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_File_write_shared_fortran_wrapper(fh, buf, count, datatype, status, ierr);
}

/* ================= End Wrappers for MPI_File_write_shared ================= */


/* ================== C Wrappers for MPI_Finalize ================== */
_EXTERN_C_ int PMPI_Finalize();
_EXTERN_C_ int MPI_Finalize() { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Finalize();
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Finalize =============== */
static void MPI_Finalize_fortran_wrapper(MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Finalize();
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FINALIZE(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize_(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize__(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Finalize ================= */


/* ================== C Wrappers for MPI_Finalized ================== */
_EXTERN_C_ int PMPI_Finalized(int *flag);
_EXTERN_C_ int MPI_Finalized(int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Finalized(flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Finalized =============== */
static void MPI_Finalized_fortran_wrapper(MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Finalized((int*)flag);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FINALIZED(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Finalized_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_finalized(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Finalized_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_finalized_(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Finalized_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_finalized__(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Finalized_fortran_wrapper(flag, ierr);
}

/* ================= End Wrappers for MPI_Finalized ================= */


/* ================== C Wrappers for MPI_Free_mem ================== */
_EXTERN_C_ int PMPI_Free_mem(void *base);
_EXTERN_C_ int MPI_Free_mem(void *base) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Free_mem(base);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Free_mem =============== */
static void MPI_Free_mem_fortran_wrapper(MPI_Fint *base, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Free_mem((void*)base);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FREE_MEM(MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Free_mem_fortran_wrapper(base, ierr);
}

_EXTERN_C_ void mpi_free_mem(MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Free_mem_fortran_wrapper(base, ierr);
}

_EXTERN_C_ void mpi_free_mem_(MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Free_mem_fortran_wrapper(base, ierr);
}

_EXTERN_C_ void mpi_free_mem__(MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Free_mem_fortran_wrapper(base, ierr);
}

/* ================= End Wrappers for MPI_Free_mem ================= */


/* ================== C Wrappers for MPI_Gather ================== */
_EXTERN_C_ int PMPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
_EXTERN_C_ int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Gather =============== */
static void MPI_Gather_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Gather((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Gather((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GATHER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_gather(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_gather_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_gather__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

/* ================= End Wrappers for MPI_Gather ================= */


/* ================== C Wrappers for MPI_Gatherv ================== */
_EXTERN_C_ int PMPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, int root, MPI_Comm comm);
_EXTERN_C_ int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, int root, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Gatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Gatherv =============== */
static void MPI_Gatherv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Gatherv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Gatherv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GATHERV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_gatherv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_gatherv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_gatherv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, ierr);
}

/* ================= End Wrappers for MPI_Gatherv ================= */


/* ================== C Wrappers for MPI_Get ================== */
_EXTERN_C_ int PMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win);
_EXTERN_C_ int MPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get =============== */
static void MPI_Get_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Get((void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Get((void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

_EXTERN_C_ void mpi_get(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

_EXTERN_C_ void mpi_get_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

_EXTERN_C_ void mpi_get__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

/* ================= End Wrappers for MPI_Get ================= */


/* ================== C Wrappers for MPI_Get_accumulate ================== */
_EXTERN_C_ int PMPI_Get_accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, void *result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win);
_EXTERN_C_ int MPI_Get_accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, void *result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_accumulate(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_accumulate =============== */
static void MPI_Get_accumulate_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Get_accumulate((const void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), (void*)result_addr, *result_count, (MPI_Datatype)(*result_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Op)(*op), (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Get_accumulate((const void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), (void*)result_addr, *result_count, MPI_Type_f2c(*result_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Op_f2c(*op), MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_ACCUMULATE(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

_EXTERN_C_ void mpi_get_accumulate(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

_EXTERN_C_ void mpi_get_accumulate_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

_EXTERN_C_ void mpi_get_accumulate__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Get_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, ierr);
}

/* ================= End Wrappers for MPI_Get_accumulate ================= */


/* ================== C Wrappers for MPI_Get_address ================== */
_EXTERN_C_ int PMPI_Get_address(const void *location, MPI_Aint *address);
_EXTERN_C_ int MPI_Get_address(const void *location, MPI_Aint *address) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_address(location, address);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_address =============== */
static void MPI_Get_address_fortran_wrapper(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Get_address((const void*)location, (MPI_Aint*)address);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_ADDRESS(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Get_address_fortran_wrapper(location, address, ierr);
}

_EXTERN_C_ void mpi_get_address(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Get_address_fortran_wrapper(location, address, ierr);
}

_EXTERN_C_ void mpi_get_address_(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Get_address_fortran_wrapper(location, address, ierr);
}

_EXTERN_C_ void mpi_get_address__(MPI_Fint *location, MPI_Aint *address, MPI_Fint *ierr) { 
    MPI_Get_address_fortran_wrapper(location, address, ierr);
}

/* ================= End Wrappers for MPI_Get_address ================= */


/* ================== C Wrappers for MPI_Get_count ================== */
_EXTERN_C_ int PMPI_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count);
_EXTERN_C_ int MPI_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_count(status, datatype, count);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_count =============== */
static void MPI_Get_count_fortran_wrapper(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Get_count((const MPI_Status*)status, (MPI_Datatype)(*datatype), (int*)count);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Get_count((const MPI_Status*)status, MPI_Type_f2c(*datatype), (int*)count);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_COUNT(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_count_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_count(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_count_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_count_(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_count_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_count__(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_count_fortran_wrapper(status, datatype, count, ierr);
}

/* ================= End Wrappers for MPI_Get_count ================= */


/* ================== C Wrappers for MPI_Get_elements ================== */
_EXTERN_C_ int PMPI_Get_elements(const MPI_Status *status, MPI_Datatype datatype, int *count);
_EXTERN_C_ int MPI_Get_elements(const MPI_Status *status, MPI_Datatype datatype, int *count) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_elements(status, datatype, count);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_elements =============== */
static void MPI_Get_elements_fortran_wrapper(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Get_elements((const MPI_Status*)status, (MPI_Datatype)(*datatype), (int*)count);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Get_elements((const MPI_Status*)status, MPI_Type_f2c(*datatype), (int*)count);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_ELEMENTS(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_elements(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_elements_(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_elements__(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_fortran_wrapper(status, datatype, count, ierr);
}

/* ================= End Wrappers for MPI_Get_elements ================= */


/* ================== C Wrappers for MPI_Get_elements_x ================== */
_EXTERN_C_ int PMPI_Get_elements_x(const MPI_Status *status, MPI_Datatype datatype, MPI_Count *count);
_EXTERN_C_ int MPI_Get_elements_x(const MPI_Status *status, MPI_Datatype datatype, MPI_Count *count) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_elements_x(status, datatype, count);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_elements_x =============== */
static void MPI_Get_elements_x_fortran_wrapper(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Get_elements_x((const MPI_Status*)status, (MPI_Datatype)(*datatype), (MPI_Count*)count);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Get_elements_x((const MPI_Status*)status, MPI_Type_f2c(*datatype), (MPI_Count*)count);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_ELEMENTS_X(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_elements_x(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_elements_x_(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_get_elements_x__(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Get_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

/* ================= End Wrappers for MPI_Get_elements_x ================= */


/* ================== C Wrappers for MPI_Get_library_version ================== */
_EXTERN_C_ int PMPI_Get_library_version(char *version, int *resultlen);
_EXTERN_C_ int MPI_Get_library_version(char *version, int *resultlen) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_library_version(version, resultlen);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_library_version =============== */
static void MPI_Get_library_version_fortran_wrapper(MPI_Fint *version, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Get_library_version((char*)version, (int*)resultlen);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_LIBRARY_VERSION(MPI_Fint *version, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_library_version_fortran_wrapper(version, resultlen, ierr);
}

_EXTERN_C_ void mpi_get_library_version(MPI_Fint *version, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_library_version_fortran_wrapper(version, resultlen, ierr);
}

_EXTERN_C_ void mpi_get_library_version_(MPI_Fint *version, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_library_version_fortran_wrapper(version, resultlen, ierr);
}

_EXTERN_C_ void mpi_get_library_version__(MPI_Fint *version, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_library_version_fortran_wrapper(version, resultlen, ierr);
}

/* ================= End Wrappers for MPI_Get_library_version ================= */


/* ================== C Wrappers for MPI_Get_processor_name ================== */
_EXTERN_C_ int PMPI_Get_processor_name(char *name, int *resultlen);
_EXTERN_C_ int MPI_Get_processor_name(char *name, int *resultlen) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_processor_name(name, resultlen);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_processor_name =============== */
static void MPI_Get_processor_name_fortran_wrapper(MPI_Fint *name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Get_processor_name((char*)name, (int*)resultlen);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_PROCESSOR_NAME(MPI_Fint *name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_processor_name_fortran_wrapper(name, resultlen, ierr);
}

_EXTERN_C_ void mpi_get_processor_name(MPI_Fint *name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_processor_name_fortran_wrapper(name, resultlen, ierr);
}

_EXTERN_C_ void mpi_get_processor_name_(MPI_Fint *name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_processor_name_fortran_wrapper(name, resultlen, ierr);
}

_EXTERN_C_ void mpi_get_processor_name__(MPI_Fint *name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Get_processor_name_fortran_wrapper(name, resultlen, ierr);
}

/* ================= End Wrappers for MPI_Get_processor_name ================= */


/* ================== C Wrappers for MPI_Get_version ================== */
_EXTERN_C_ int PMPI_Get_version(int *version, int *subversion);
_EXTERN_C_ int MPI_Get_version(int *version, int *subversion) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Get_version(version, subversion);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Get_version =============== */
static void MPI_Get_version_fortran_wrapper(MPI_Fint *version, MPI_Fint *subversion, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Get_version((int*)version, (int*)subversion);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GET_VERSION(MPI_Fint *version, MPI_Fint *subversion, MPI_Fint *ierr) { 
    MPI_Get_version_fortran_wrapper(version, subversion, ierr);
}

_EXTERN_C_ void mpi_get_version(MPI_Fint *version, MPI_Fint *subversion, MPI_Fint *ierr) { 
    MPI_Get_version_fortran_wrapper(version, subversion, ierr);
}

_EXTERN_C_ void mpi_get_version_(MPI_Fint *version, MPI_Fint *subversion, MPI_Fint *ierr) { 
    MPI_Get_version_fortran_wrapper(version, subversion, ierr);
}

_EXTERN_C_ void mpi_get_version__(MPI_Fint *version, MPI_Fint *subversion, MPI_Fint *ierr) { 
    MPI_Get_version_fortran_wrapper(version, subversion, ierr);
}

/* ================= End Wrappers for MPI_Get_version ================= */


/* ================== C Wrappers for MPI_Graph_create ================== */
_EXTERN_C_ int PMPI_Graph_create(MPI_Comm comm_old, int nnodes, const int index[], const int edges[], int reorder, MPI_Comm *comm_graph);
_EXTERN_C_ int MPI_Graph_create(MPI_Comm comm_old, int nnodes, const int index[], const int edges[], int reorder, MPI_Comm *comm_graph) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm_old);

   _wrap_py_return_val = PMPI_Graph_create(comm_old, nnodes, index, edges, reorder, comm_graph);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Graph_create =============== */
static void MPI_Graph_create_fortran_wrapper(MPI_Fint *comm_old, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *reorder, MPI_Fint *comm_graph, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Graph_create((MPI_Comm)(*comm_old), *nnodes, (const int*)index, (const int*)edges, *reorder, (MPI_Comm*)comm_graph);
#else /* MPI-2 safe call */
    MPI_Comm temp_comm_graph;
    temp_comm_graph = MPI_Comm_f2c(*comm_graph);
    _wrap_py_return_val = MPI_Graph_create(MPI_Comm_f2c(*comm_old), *nnodes, (const int*)index, (const int*)edges, *reorder, &temp_comm_graph);
    *comm_graph = MPI_Comm_c2f(temp_comm_graph);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GRAPH_CREATE(MPI_Fint *comm_old, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *reorder, MPI_Fint *comm_graph, MPI_Fint *ierr) { 
    MPI_Graph_create_fortran_wrapper(comm_old, nnodes, index, edges, reorder, comm_graph, ierr);
}

_EXTERN_C_ void mpi_graph_create(MPI_Fint *comm_old, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *reorder, MPI_Fint *comm_graph, MPI_Fint *ierr) { 
    MPI_Graph_create_fortran_wrapper(comm_old, nnodes, index, edges, reorder, comm_graph, ierr);
}

_EXTERN_C_ void mpi_graph_create_(MPI_Fint *comm_old, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *reorder, MPI_Fint *comm_graph, MPI_Fint *ierr) { 
    MPI_Graph_create_fortran_wrapper(comm_old, nnodes, index, edges, reorder, comm_graph, ierr);
}

_EXTERN_C_ void mpi_graph_create__(MPI_Fint *comm_old, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *reorder, MPI_Fint *comm_graph, MPI_Fint *ierr) { 
    MPI_Graph_create_fortran_wrapper(comm_old, nnodes, index, edges, reorder, comm_graph, ierr);
}

/* ================= End Wrappers for MPI_Graph_create ================= */


/* ================== C Wrappers for MPI_Graph_get ================== */
_EXTERN_C_ int PMPI_Graph_get(MPI_Comm comm, int maxindex, int maxedges, int index[], int edges[]);
_EXTERN_C_ int MPI_Graph_get(MPI_Comm comm, int maxindex, int maxedges, int index[], int edges[]) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Graph_get(comm, maxindex, maxedges, index, edges);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Graph_get =============== */
static void MPI_Graph_get_fortran_wrapper(MPI_Fint *comm, MPI_Fint *maxindex, MPI_Fint *maxedges, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Graph_get((MPI_Comm)(*comm), *maxindex, *maxedges, (int*)index, (int*)edges);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Graph_get(MPI_Comm_f2c(*comm), *maxindex, *maxedges, (int*)index, (int*)edges);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GRAPH_GET(MPI_Fint *comm, MPI_Fint *maxindex, MPI_Fint *maxedges, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *ierr) { 
    MPI_Graph_get_fortran_wrapper(comm, maxindex, maxedges, index, edges, ierr);
}

_EXTERN_C_ void mpi_graph_get(MPI_Fint *comm, MPI_Fint *maxindex, MPI_Fint *maxedges, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *ierr) { 
    MPI_Graph_get_fortran_wrapper(comm, maxindex, maxedges, index, edges, ierr);
}

_EXTERN_C_ void mpi_graph_get_(MPI_Fint *comm, MPI_Fint *maxindex, MPI_Fint *maxedges, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *ierr) { 
    MPI_Graph_get_fortran_wrapper(comm, maxindex, maxedges, index, edges, ierr);
}

_EXTERN_C_ void mpi_graph_get__(MPI_Fint *comm, MPI_Fint *maxindex, MPI_Fint *maxedges, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *ierr) { 
    MPI_Graph_get_fortran_wrapper(comm, maxindex, maxedges, index, edges, ierr);
}

/* ================= End Wrappers for MPI_Graph_get ================= */


/* ================== C Wrappers for MPI_Graph_map ================== */
_EXTERN_C_ int PMPI_Graph_map(MPI_Comm comm, int nnodes, const int index[], const int edges[], int *newrank);
_EXTERN_C_ int MPI_Graph_map(MPI_Comm comm, int nnodes, const int index[], const int edges[], int *newrank) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Graph_map(comm, nnodes, index, edges, newrank);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Graph_map =============== */
static void MPI_Graph_map_fortran_wrapper(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Graph_map((MPI_Comm)(*comm), *nnodes, (const int*)index, (const int*)edges, (int*)newrank);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Graph_map(MPI_Comm_f2c(*comm), *nnodes, (const int*)index, (const int*)edges, (int*)newrank);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GRAPH_MAP(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Graph_map_fortran_wrapper(comm, nnodes, index, edges, newrank, ierr);
}

_EXTERN_C_ void mpi_graph_map(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Graph_map_fortran_wrapper(comm, nnodes, index, edges, newrank, ierr);
}

_EXTERN_C_ void mpi_graph_map_(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Graph_map_fortran_wrapper(comm, nnodes, index, edges, newrank, ierr);
}

_EXTERN_C_ void mpi_graph_map__(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint index[], MPI_Fint edges[], MPI_Fint *newrank, MPI_Fint *ierr) { 
    MPI_Graph_map_fortran_wrapper(comm, nnodes, index, edges, newrank, ierr);
}

/* ================= End Wrappers for MPI_Graph_map ================= */


/* ================== C Wrappers for MPI_Graph_neighbors ================== */
_EXTERN_C_ int PMPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors, int neighbors[]);
_EXTERN_C_ int MPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors, int neighbors[]) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Graph_neighbors(comm, rank, maxneighbors, neighbors);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Graph_neighbors =============== */
static void MPI_Graph_neighbors_fortran_wrapper(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxneighbors, MPI_Fint neighbors[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Graph_neighbors((MPI_Comm)(*comm), *rank, *maxneighbors, (int*)neighbors);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Graph_neighbors(MPI_Comm_f2c(*comm), *rank, *maxneighbors, (int*)neighbors);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GRAPH_NEIGHBORS(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxneighbors, MPI_Fint neighbors[], MPI_Fint *ierr) { 
    MPI_Graph_neighbors_fortran_wrapper(comm, rank, maxneighbors, neighbors, ierr);
}

_EXTERN_C_ void mpi_graph_neighbors(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxneighbors, MPI_Fint neighbors[], MPI_Fint *ierr) { 
    MPI_Graph_neighbors_fortran_wrapper(comm, rank, maxneighbors, neighbors, ierr);
}

_EXTERN_C_ void mpi_graph_neighbors_(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxneighbors, MPI_Fint neighbors[], MPI_Fint *ierr) { 
    MPI_Graph_neighbors_fortran_wrapper(comm, rank, maxneighbors, neighbors, ierr);
}

_EXTERN_C_ void mpi_graph_neighbors__(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *maxneighbors, MPI_Fint neighbors[], MPI_Fint *ierr) { 
    MPI_Graph_neighbors_fortran_wrapper(comm, rank, maxneighbors, neighbors, ierr);
}

/* ================= End Wrappers for MPI_Graph_neighbors ================= */


/* ================== C Wrappers for MPI_Graph_neighbors_count ================== */
_EXTERN_C_ int PMPI_Graph_neighbors_count(MPI_Comm comm, int rank, int *nneighbors);
_EXTERN_C_ int MPI_Graph_neighbors_count(MPI_Comm comm, int rank, int *nneighbors) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Graph_neighbors_count(comm, rank, nneighbors);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Graph_neighbors_count =============== */
static void MPI_Graph_neighbors_count_fortran_wrapper(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *nneighbors, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Graph_neighbors_count((MPI_Comm)(*comm), *rank, (int*)nneighbors);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Graph_neighbors_count(MPI_Comm_f2c(*comm), *rank, (int*)nneighbors);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GRAPH_NEIGHBORS_COUNT(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *nneighbors, MPI_Fint *ierr) { 
    MPI_Graph_neighbors_count_fortran_wrapper(comm, rank, nneighbors, ierr);
}

_EXTERN_C_ void mpi_graph_neighbors_count(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *nneighbors, MPI_Fint *ierr) { 
    MPI_Graph_neighbors_count_fortran_wrapper(comm, rank, nneighbors, ierr);
}

_EXTERN_C_ void mpi_graph_neighbors_count_(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *nneighbors, MPI_Fint *ierr) { 
    MPI_Graph_neighbors_count_fortran_wrapper(comm, rank, nneighbors, ierr);
}

_EXTERN_C_ void mpi_graph_neighbors_count__(MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *nneighbors, MPI_Fint *ierr) { 
    MPI_Graph_neighbors_count_fortran_wrapper(comm, rank, nneighbors, ierr);
}

/* ================= End Wrappers for MPI_Graph_neighbors_count ================= */


/* ================== C Wrappers for MPI_Graphdims_get ================== */
_EXTERN_C_ int PMPI_Graphdims_get(MPI_Comm comm, int *nnodes, int *nedges);
_EXTERN_C_ int MPI_Graphdims_get(MPI_Comm comm, int *nnodes, int *nedges) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Graphdims_get(comm, nnodes, nedges);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Graphdims_get =============== */
static void MPI_Graphdims_get_fortran_wrapper(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint *nedges, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Graphdims_get((MPI_Comm)(*comm), (int*)nnodes, (int*)nedges);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Graphdims_get(MPI_Comm_f2c(*comm), (int*)nnodes, (int*)nedges);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GRAPHDIMS_GET(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint *nedges, MPI_Fint *ierr) { 
    MPI_Graphdims_get_fortran_wrapper(comm, nnodes, nedges, ierr);
}

_EXTERN_C_ void mpi_graphdims_get(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint *nedges, MPI_Fint *ierr) { 
    MPI_Graphdims_get_fortran_wrapper(comm, nnodes, nedges, ierr);
}

_EXTERN_C_ void mpi_graphdims_get_(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint *nedges, MPI_Fint *ierr) { 
    MPI_Graphdims_get_fortran_wrapper(comm, nnodes, nedges, ierr);
}

_EXTERN_C_ void mpi_graphdims_get__(MPI_Fint *comm, MPI_Fint *nnodes, MPI_Fint *nedges, MPI_Fint *ierr) { 
    MPI_Graphdims_get_fortran_wrapper(comm, nnodes, nedges, ierr);
}

/* ================= End Wrappers for MPI_Graphdims_get ================= */


/* ================== C Wrappers for MPI_Grequest_complete ================== */
_EXTERN_C_ int PMPI_Grequest_complete(MPI_Request request);
_EXTERN_C_ int MPI_Grequest_complete(MPI_Request request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Grequest_complete(request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Grequest_complete =============== */
static void MPI_Grequest_complete_fortran_wrapper(MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Grequest_complete((MPI_Request)(*request));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Grequest_complete(MPI_Request_f2c(*request));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GREQUEST_COMPLETE(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_complete_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_grequest_complete(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_complete_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_grequest_complete_(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_complete_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_grequest_complete__(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_complete_fortran_wrapper(request, ierr);
}

/* ================= End Wrappers for MPI_Grequest_complete ================= */


/* ================== C Wrappers for MPI_Grequest_start ================== */
_EXTERN_C_ int PMPI_Grequest_start(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, void *extra_state, MPI_Request *request);
_EXTERN_C_ int MPI_Grequest_start(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, void *extra_state, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Grequest_start(query_fn, free_fn, cancel_fn, extra_state, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Grequest_start =============== */
static void MPI_Grequest_start_fortran_wrapper(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, MPI_Fint *extra_state, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Grequest_start((MPI_Grequest_query_function*)query_fn, (MPI_Grequest_free_function*)free_fn, (MPI_Grequest_cancel_function*)cancel_fn, (void*)extra_state, (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Grequest_start((MPI_Grequest_query_function*)query_fn, (MPI_Grequest_free_function*)free_fn, (MPI_Grequest_cancel_function*)cancel_fn, (void*)extra_state, &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GREQUEST_START(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, MPI_Fint *extra_state, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_start_fortran_wrapper(query_fn, free_fn, cancel_fn, extra_state, request, ierr);
}

_EXTERN_C_ void mpi_grequest_start(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, MPI_Fint *extra_state, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_start_fortran_wrapper(query_fn, free_fn, cancel_fn, extra_state, request, ierr);
}

_EXTERN_C_ void mpi_grequest_start_(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, MPI_Fint *extra_state, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_start_fortran_wrapper(query_fn, free_fn, cancel_fn, extra_state, request, ierr);
}

_EXTERN_C_ void mpi_grequest_start__(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, MPI_Fint *extra_state, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Grequest_start_fortran_wrapper(query_fn, free_fn, cancel_fn, extra_state, request, ierr);
}

/* ================= End Wrappers for MPI_Grequest_start ================= */


/* ================== C Wrappers for MPI_Group_compare ================== */
_EXTERN_C_ int PMPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result);
_EXTERN_C_ int MPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_compare(group1, group2, result);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_compare =============== */
static void MPI_Group_compare_fortran_wrapper(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *result, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_compare((MPI_Group)(*group1), (MPI_Group)(*group2), (int*)result);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Group_compare(MPI_Group_f2c(*group1), MPI_Group_f2c(*group2), (int*)result);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_COMPARE(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Group_compare_fortran_wrapper(group1, group2, result, ierr);
}

_EXTERN_C_ void mpi_group_compare(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Group_compare_fortran_wrapper(group1, group2, result, ierr);
}

_EXTERN_C_ void mpi_group_compare_(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Group_compare_fortran_wrapper(group1, group2, result, ierr);
}

_EXTERN_C_ void mpi_group_compare__(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *result, MPI_Fint *ierr) { 
    MPI_Group_compare_fortran_wrapper(group1, group2, result, ierr);
}

/* ================= End Wrappers for MPI_Group_compare ================= */


/* ================== C Wrappers for MPI_Group_difference ================== */
_EXTERN_C_ int PMPI_Group_difference(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_difference(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_difference(group1, group2, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_difference =============== */
static void MPI_Group_difference_fortran_wrapper(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_difference((MPI_Group)(*group1), (MPI_Group)(*group2), (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_difference(MPI_Group_f2c(*group1), MPI_Group_f2c(*group2), &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_DIFFERENCE(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_difference_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_difference(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_difference_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_difference_(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_difference_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_difference__(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_difference_fortran_wrapper(group1, group2, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_difference ================= */


/* ================== C Wrappers for MPI_Group_excl ================== */
_EXTERN_C_ int PMPI_Group_excl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_excl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_excl(group, n, ranks, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_excl =============== */
static void MPI_Group_excl_fortran_wrapper(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_excl((MPI_Group)(*group), *n, (const int*)ranks, (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_excl(MPI_Group_f2c(*group), *n, (const int*)ranks, &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_EXCL(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_excl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_excl(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_excl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_excl_(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_excl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_excl__(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_excl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_excl ================= */


/* ================== C Wrappers for MPI_Group_free ================== */
_EXTERN_C_ int PMPI_Group_free(MPI_Group *group);
_EXTERN_C_ int MPI_Group_free(MPI_Group *group) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_free(group);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_free =============== */
static void MPI_Group_free_fortran_wrapper(MPI_Fint *group, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_free((MPI_Group*)group);
#else /* MPI-2 safe call */
    MPI_Group temp_group;
    temp_group = MPI_Group_f2c(*group);
    _wrap_py_return_val = MPI_Group_free(&temp_group);
    *group = MPI_Group_c2f(temp_group);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_FREE(MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Group_free_fortran_wrapper(group, ierr);
}

_EXTERN_C_ void mpi_group_free(MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Group_free_fortran_wrapper(group, ierr);
}

_EXTERN_C_ void mpi_group_free_(MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Group_free_fortran_wrapper(group, ierr);
}

_EXTERN_C_ void mpi_group_free__(MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Group_free_fortran_wrapper(group, ierr);
}

/* ================= End Wrappers for MPI_Group_free ================= */


/* ================== C Wrappers for MPI_Group_incl ================== */
_EXTERN_C_ int PMPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_incl(group, n, ranks, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_incl =============== */
static void MPI_Group_incl_fortran_wrapper(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_incl((MPI_Group)(*group), *n, (const int*)ranks, (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_incl(MPI_Group_f2c(*group), *n, (const int*)ranks, &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_INCL(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_incl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_incl(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_incl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_incl_(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_incl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_incl__(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranks[], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_incl_fortran_wrapper(group, n, ranks, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_incl ================= */


/* ================== C Wrappers for MPI_Group_intersection ================== */
_EXTERN_C_ int PMPI_Group_intersection(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_intersection(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_intersection(group1, group2, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_intersection =============== */
static void MPI_Group_intersection_fortran_wrapper(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_intersection((MPI_Group)(*group1), (MPI_Group)(*group2), (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_intersection(MPI_Group_f2c(*group1), MPI_Group_f2c(*group2), &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_INTERSECTION(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_intersection_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_intersection(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_intersection_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_intersection_(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_intersection_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_intersection__(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_intersection_fortran_wrapper(group1, group2, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_intersection ================= */


/* ================== C Wrappers for MPI_Group_range_excl ================== */
_EXTERN_C_ int PMPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_range_excl(group, n, ranges, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_range_excl =============== */
static void MPI_Group_range_excl_fortran_wrapper(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_range_excl((MPI_Group)(*group), *n, (int(*)[3])ranges, (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_range_excl(MPI_Group_f2c(*group), *n, (int(*)[3])ranges, &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_RANGE_EXCL(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_excl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_range_excl(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_excl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_range_excl_(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_excl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_range_excl__(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_excl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_range_excl ================= */


/* ================== C Wrappers for MPI_Group_range_incl ================== */
_EXTERN_C_ int PMPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_range_incl(group, n, ranges, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_range_incl =============== */
static void MPI_Group_range_incl_fortran_wrapper(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_range_incl((MPI_Group)(*group), *n, (int(*)[3])ranges, (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_range_incl(MPI_Group_f2c(*group), *n, (int(*)[3])ranges, &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_RANGE_INCL(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_incl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_range_incl(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_incl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_range_incl_(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_incl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_range_incl__(MPI_Fint *group, MPI_Fint *n, MPI_Fint ranges[][3], MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_range_incl_fortran_wrapper(group, n, ranges, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_range_incl ================= */


/* ================== C Wrappers for MPI_Group_rank ================== */
_EXTERN_C_ int PMPI_Group_rank(MPI_Group group, int *rank);
_EXTERN_C_ int MPI_Group_rank(MPI_Group group, int *rank) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_rank(group, rank);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_rank =============== */
static void MPI_Group_rank_fortran_wrapper(MPI_Fint *group, MPI_Fint *rank, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_rank((MPI_Group)(*group), (int*)rank);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Group_rank(MPI_Group_f2c(*group), (int*)rank);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_RANK(MPI_Fint *group, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Group_rank_fortran_wrapper(group, rank, ierr);
}

_EXTERN_C_ void mpi_group_rank(MPI_Fint *group, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Group_rank_fortran_wrapper(group, rank, ierr);
}

_EXTERN_C_ void mpi_group_rank_(MPI_Fint *group, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Group_rank_fortran_wrapper(group, rank, ierr);
}

_EXTERN_C_ void mpi_group_rank__(MPI_Fint *group, MPI_Fint *rank, MPI_Fint *ierr) { 
    MPI_Group_rank_fortran_wrapper(group, rank, ierr);
}

/* ================= End Wrappers for MPI_Group_rank ================= */


/* ================== C Wrappers for MPI_Group_size ================== */
_EXTERN_C_ int PMPI_Group_size(MPI_Group group, int *size);
_EXTERN_C_ int MPI_Group_size(MPI_Group group, int *size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_size(group, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_size =============== */
static void MPI_Group_size_fortran_wrapper(MPI_Fint *group, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_size((MPI_Group)(*group), (int*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Group_size(MPI_Group_f2c(*group), (int*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_SIZE(MPI_Fint *group, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Group_size_fortran_wrapper(group, size, ierr);
}

_EXTERN_C_ void mpi_group_size(MPI_Fint *group, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Group_size_fortran_wrapper(group, size, ierr);
}

_EXTERN_C_ void mpi_group_size_(MPI_Fint *group, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Group_size_fortran_wrapper(group, size, ierr);
}

_EXTERN_C_ void mpi_group_size__(MPI_Fint *group, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Group_size_fortran_wrapper(group, size, ierr);
}

/* ================= End Wrappers for MPI_Group_size ================= */


/* ================== C Wrappers for MPI_Group_translate_ranks ================== */
_EXTERN_C_ int PMPI_Group_translate_ranks(MPI_Group group1, int n, const int ranks1[], MPI_Group group2, int ranks2[]);
_EXTERN_C_ int MPI_Group_translate_ranks(MPI_Group group1, int n, const int ranks1[], MPI_Group group2, int ranks2[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_translate_ranks(group1, n, ranks1, group2, ranks2);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_translate_ranks =============== */
static void MPI_Group_translate_ranks_fortran_wrapper(MPI_Fint *group1, MPI_Fint *n, MPI_Fint ranks1[], MPI_Fint *group2, MPI_Fint ranks2[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_translate_ranks((MPI_Group)(*group1), *n, (const int*)ranks1, (MPI_Group)(*group2), (int*)ranks2);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Group_translate_ranks(MPI_Group_f2c(*group1), *n, (const int*)ranks1, MPI_Group_f2c(*group2), (int*)ranks2);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_TRANSLATE_RANKS(MPI_Fint *group1, MPI_Fint *n, MPI_Fint ranks1[], MPI_Fint *group2, MPI_Fint ranks2[], MPI_Fint *ierr) { 
    MPI_Group_translate_ranks_fortran_wrapper(group1, n, ranks1, group2, ranks2, ierr);
}

_EXTERN_C_ void mpi_group_translate_ranks(MPI_Fint *group1, MPI_Fint *n, MPI_Fint ranks1[], MPI_Fint *group2, MPI_Fint ranks2[], MPI_Fint *ierr) { 
    MPI_Group_translate_ranks_fortran_wrapper(group1, n, ranks1, group2, ranks2, ierr);
}

_EXTERN_C_ void mpi_group_translate_ranks_(MPI_Fint *group1, MPI_Fint *n, MPI_Fint ranks1[], MPI_Fint *group2, MPI_Fint ranks2[], MPI_Fint *ierr) { 
    MPI_Group_translate_ranks_fortran_wrapper(group1, n, ranks1, group2, ranks2, ierr);
}

_EXTERN_C_ void mpi_group_translate_ranks__(MPI_Fint *group1, MPI_Fint *n, MPI_Fint ranks1[], MPI_Fint *group2, MPI_Fint ranks2[], MPI_Fint *ierr) { 
    MPI_Group_translate_ranks_fortran_wrapper(group1, n, ranks1, group2, ranks2, ierr);
}

/* ================= End Wrappers for MPI_Group_translate_ranks ================= */


/* ================== C Wrappers for MPI_Group_union ================== */
_EXTERN_C_ int PMPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
_EXTERN_C_ int MPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Group_union(group1, group2, newgroup);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Group_union =============== */
static void MPI_Group_union_fortran_wrapper(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Group_union((MPI_Group)(*group1), (MPI_Group)(*group2), (MPI_Group*)newgroup);
#else /* MPI-2 safe call */
    MPI_Group temp_newgroup;
    temp_newgroup = MPI_Group_f2c(*newgroup);
    _wrap_py_return_val = MPI_Group_union(MPI_Group_f2c(*group1), MPI_Group_f2c(*group2), &temp_newgroup);
    *newgroup = MPI_Group_c2f(temp_newgroup);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_GROUP_UNION(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_union_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_union(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_union_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_union_(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_union_fortran_wrapper(group1, group2, newgroup, ierr);
}

_EXTERN_C_ void mpi_group_union__(MPI_Fint *group1, MPI_Fint *group2, MPI_Fint *newgroup, MPI_Fint *ierr) { 
    MPI_Group_union_fortran_wrapper(group1, group2, newgroup, ierr);
}

/* ================= End Wrappers for MPI_Group_union ================= */


/* ================== C Wrappers for MPI_Iallgather ================== */
_EXTERN_C_ int PMPI_Iallgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iallgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iallgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iallgather =============== */
static void MPI_Iallgather_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iallgather((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iallgather((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IALLGATHER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallgather(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallgather_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallgather__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iallgather ================= */


/* ================== C Wrappers for MPI_Iallgatherv ================== */
_EXTERN_C_ int PMPI_Iallgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iallgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iallgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iallgatherv =============== */
static void MPI_Iallgatherv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iallgatherv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iallgatherv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IALLGATHERV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallgatherv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallgatherv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallgatherv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iallgatherv ================= */


/* ================== C Wrappers for MPI_Iallreduce ================== */
_EXTERN_C_ int PMPI_Iallreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iallreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iallreduce(sendbuf, recvbuf, count, datatype, op, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iallreduce =============== */
static void MPI_Iallreduce_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iallreduce((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iallreduce((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IALLREDUCE(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallreduce(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallreduce_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iallreduce__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iallreduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iallreduce ================= */


/* ================== C Wrappers for MPI_Ialltoall ================== */
_EXTERN_C_ int PMPI_Ialltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ialltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ialltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ialltoall =============== */
static void MPI_Ialltoall_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ialltoall((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ialltoall((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IALLTOALL(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoall(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoall_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoall__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ialltoall ================= */


/* ================== C Wrappers for MPI_Ialltoallv ================== */
_EXTERN_C_ int PMPI_Ialltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ialltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ialltoallv(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ialltoallv =============== */
static void MPI_Ialltoallv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ialltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ialltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IALLTOALLV(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoallv(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoallv_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoallv__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ialltoallv ================= */


/* ================== C Wrappers for MPI_Ialltoallw ================== */
_EXTERN_C_ int PMPI_Ialltoallw(const void *sendbuf, const int sendcounts[], const int sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const int rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ialltoallw(const void *sendbuf, const int sendcounts[], const int sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const int rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ialltoallw(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ialltoallw =============== */
static void MPI_Ialltoallw_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ialltoallw((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (const MPI_Datatype*)recvtypes, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ialltoallw((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (const MPI_Datatype*)recvtypes, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IALLTOALLW(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoallw(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoallw_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

_EXTERN_C_ void mpi_ialltoallw__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ialltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ialltoallw ================= */


/* ================== C Wrappers for MPI_Ibarrier ================== */
_EXTERN_C_ int PMPI_Ibarrier(MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ibarrier(MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ibarrier(comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ibarrier =============== */
static void MPI_Ibarrier_fortran_wrapper(MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ibarrier((MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ibarrier(MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IBARRIER(MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibarrier_fortran_wrapper(comm, request, ierr);
}

_EXTERN_C_ void mpi_ibarrier(MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibarrier_fortran_wrapper(comm, request, ierr);
}

_EXTERN_C_ void mpi_ibarrier_(MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibarrier_fortran_wrapper(comm, request, ierr);
}

_EXTERN_C_ void mpi_ibarrier__(MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibarrier_fortran_wrapper(comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ibarrier ================= */


/* ================== C Wrappers for MPI_Ibcast ================== */
_EXTERN_C_ int PMPI_Ibcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ibcast(buffer, count, datatype, root, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ibcast =============== */
static void MPI_Ibcast_fortran_wrapper(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ibcast((void*)buffer, *count, (MPI_Datatype)(*datatype), *root, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ibcast((void*)buffer, *count, MPI_Type_f2c(*datatype), *root, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IBCAST(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibcast_fortran_wrapper(buffer, count, datatype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_ibcast(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibcast_fortran_wrapper(buffer, count, datatype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_ibcast_(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibcast_fortran_wrapper(buffer, count, datatype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_ibcast__(MPI_Fint *buffer, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibcast_fortran_wrapper(buffer, count, datatype, root, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ibcast ================= */


/* ================== C Wrappers for MPI_Ibsend ================== */
_EXTERN_C_ int PMPI_Ibsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ibsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ibsend(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ibsend =============== */
static void MPI_Ibsend_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ibsend((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ibsend((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IBSEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_ibsend(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_ibsend_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_ibsend__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ibsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ibsend ================= */


/* ================== C Wrappers for MPI_Iexscan ================== */
_EXTERN_C_ int PMPI_Iexscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iexscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iexscan(sendbuf, recvbuf, count, datatype, op, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iexscan =============== */
static void MPI_Iexscan_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iexscan((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iexscan((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IEXSCAN(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iexscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iexscan(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iexscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iexscan_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iexscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iexscan__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iexscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iexscan ================= */


/* ================== C Wrappers for MPI_Igather ================== */
_EXTERN_C_ int PMPI_Igather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Igather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Igather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Igather =============== */
static void MPI_Igather_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Igather((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Igather((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IGATHER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_igather(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_igather_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_igather__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Igather ================= */


/* ================== C Wrappers for MPI_Igatherv ================== */
_EXTERN_C_ int PMPI_Igatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Igatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Igatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Igatherv =============== */
static void MPI_Igatherv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Igatherv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Igatherv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IGATHERV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_igatherv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_igatherv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_igatherv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Igatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Igatherv ================= */


/* ================== C Wrappers for MPI_Improbe ================== */
_EXTERN_C_ int PMPI_Improbe(int source, int tag, MPI_Comm comm, int *flag, MPI_Message *message, MPI_Status *status);
_EXTERN_C_ int MPI_Improbe(int source, int tag, MPI_Comm comm, int *flag, MPI_Message *message, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Improbe(source, tag, comm, flag, message, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Improbe =============== */
static void MPI_Improbe_fortran_wrapper(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Improbe(*source, *tag, (MPI_Comm)(*comm), (int*)flag, (MPI_Message*)message, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Improbe(*source, *tag, MPI_Comm_f2c(*comm), (int*)flag, (MPI_Message*)message, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IMPROBE(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Improbe_fortran_wrapper(source, tag, comm, flag, message, status, ierr);
}

_EXTERN_C_ void mpi_improbe(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Improbe_fortran_wrapper(source, tag, comm, flag, message, status, ierr);
}

_EXTERN_C_ void mpi_improbe_(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Improbe_fortran_wrapper(source, tag, comm, flag, message, status, ierr);
}

_EXTERN_C_ void mpi_improbe__(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Improbe_fortran_wrapper(source, tag, comm, flag, message, status, ierr);
}

/* ================= End Wrappers for MPI_Improbe ================= */


/* ================== C Wrappers for MPI_Imrecv ================== */
_EXTERN_C_ int PMPI_Imrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Request *request);
_EXTERN_C_ int MPI_Imrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Imrecv(buf, count, type, message, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Imrecv =============== */
static void MPI_Imrecv_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Imrecv((void*)buf, *count, (MPI_Datatype)(*type), (MPI_Message*)message, (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Imrecv((void*)buf, *count, MPI_Type_f2c(*type), (MPI_Message*)message, &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IMRECV(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Imrecv_fortran_wrapper(buf, count, type, message, request, ierr);
}

_EXTERN_C_ void mpi_imrecv(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Imrecv_fortran_wrapper(buf, count, type, message, request, ierr);
}

_EXTERN_C_ void mpi_imrecv_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Imrecv_fortran_wrapper(buf, count, type, message, request, ierr);
}

_EXTERN_C_ void mpi_imrecv__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Imrecv_fortran_wrapper(buf, count, type, message, request, ierr);
}

/* ================= End Wrappers for MPI_Imrecv ================= */


/* ================== C Wrappers for MPI_Ineighbor_allgather ================== */
_EXTERN_C_ int PMPI_Ineighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ineighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ineighbor_allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ineighbor_allgather =============== */
static void MPI_Ineighbor_allgather_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ineighbor_allgather((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ineighbor_allgather((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INEIGHBOR_ALLGATHER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_allgather(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_allgather_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_allgather__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ineighbor_allgather ================= */


/* ================== C Wrappers for MPI_Ineighbor_allgatherv ================== */
_EXTERN_C_ int PMPI_Ineighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ineighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ineighbor_allgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ineighbor_allgatherv =============== */
static void MPI_Ineighbor_allgatherv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ineighbor_allgatherv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ineighbor_allgatherv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INEIGHBOR_ALLGATHERV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_allgatherv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_allgatherv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_allgatherv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ineighbor_allgatherv ================= */


/* ================== C Wrappers for MPI_Ineighbor_alltoall ================== */
_EXTERN_C_ int PMPI_Ineighbor_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ineighbor_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ineighbor_alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ineighbor_alltoall =============== */
static void MPI_Ineighbor_alltoall_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ineighbor_alltoall((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ineighbor_alltoall((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INEIGHBOR_ALLTOALL(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoall(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoall_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoall__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ineighbor_alltoall ================= */


/* ================== C Wrappers for MPI_Ineighbor_alltoallv ================== */
_EXTERN_C_ int PMPI_Ineighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ineighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ineighbor_alltoallv(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ineighbor_alltoallv =============== */
static void MPI_Ineighbor_alltoallv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ineighbor_alltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ineighbor_alltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INEIGHBOR_ALLTOALLV(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoallv(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoallv_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoallv__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ineighbor_alltoallv ================= */


/* ================== C Wrappers for MPI_Ineighbor_alltoallw ================== */
_EXTERN_C_ int PMPI_Ineighbor_alltoallw(const void *sendbuf, const int sendcounts[], const MPI_Aint sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const MPI_Aint rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ineighbor_alltoallw(const void *sendbuf, const int sendcounts[], const MPI_Aint sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const MPI_Aint rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ineighbor_alltoallw(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ineighbor_alltoallw =============== */
static void MPI_Ineighbor_alltoallw_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ineighbor_alltoallw((const void*)sendbuf, (const int*)sendcounts, (const MPI_Aint*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const MPI_Aint*)rdispls, (const MPI_Datatype*)recvtypes, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ineighbor_alltoallw((const void*)sendbuf, (const int*)sendcounts, (const MPI_Aint*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const MPI_Aint*)rdispls, (const MPI_Datatype*)recvtypes, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INEIGHBOR_ALLTOALLW(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoallw(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoallw_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

_EXTERN_C_ void mpi_ineighbor_alltoallw__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ineighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ineighbor_alltoallw ================= */


/* ================== C Wrappers for MPI_Info_create ================== */
_EXTERN_C_ int PMPI_Info_create(MPI_Info *info);
_EXTERN_C_ int MPI_Info_create(MPI_Info *info) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_create(info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_create =============== */
static void MPI_Info_create_fortran_wrapper(MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_create((MPI_Info*)info);
#else /* MPI-2 safe call */
    MPI_Info temp_info;
    temp_info = MPI_Info_f2c(*info);
    _wrap_py_return_val = MPI_Info_create(&temp_info);
    *info = MPI_Info_c2f(temp_info);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_CREATE(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_create_fortran_wrapper(info, ierr);
}

_EXTERN_C_ void mpi_info_create(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_create_fortran_wrapper(info, ierr);
}

_EXTERN_C_ void mpi_info_create_(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_create_fortran_wrapper(info, ierr);
}

_EXTERN_C_ void mpi_info_create__(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_create_fortran_wrapper(info, ierr);
}

/* ================= End Wrappers for MPI_Info_create ================= */


/* ================== C Wrappers for MPI_Info_delete ================== */
_EXTERN_C_ int PMPI_Info_delete(MPI_Info info, const char *key);
_EXTERN_C_ int MPI_Info_delete(MPI_Info info, const char *key) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_delete(info, key);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_delete =============== */
static void MPI_Info_delete_fortran_wrapper(MPI_Fint *info, MPI_Fint *key, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_delete((MPI_Info)(*info), (const char*)key);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Info_delete(MPI_Info_f2c(*info), (const char*)key);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_DELETE(MPI_Fint *info, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_delete_fortran_wrapper(info, key, ierr);
}

_EXTERN_C_ void mpi_info_delete(MPI_Fint *info, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_delete_fortran_wrapper(info, key, ierr);
}

_EXTERN_C_ void mpi_info_delete_(MPI_Fint *info, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_delete_fortran_wrapper(info, key, ierr);
}

_EXTERN_C_ void mpi_info_delete__(MPI_Fint *info, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_delete_fortran_wrapper(info, key, ierr);
}

/* ================= End Wrappers for MPI_Info_delete ================= */


/* ================== C Wrappers for MPI_Info_dup ================== */
_EXTERN_C_ int PMPI_Info_dup(MPI_Info info, MPI_Info *newinfo);
_EXTERN_C_ int MPI_Info_dup(MPI_Info info, MPI_Info *newinfo) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_dup(info, newinfo);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_dup =============== */
static void MPI_Info_dup_fortran_wrapper(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_dup((MPI_Info)(*info), (MPI_Info*)newinfo);
#else /* MPI-2 safe call */
    MPI_Info temp_newinfo;
    temp_newinfo = MPI_Info_f2c(*newinfo);
    _wrap_py_return_val = MPI_Info_dup(MPI_Info_f2c(*info), &temp_newinfo);
    *newinfo = MPI_Info_c2f(temp_newinfo);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_DUP(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *ierr) { 
    MPI_Info_dup_fortran_wrapper(info, newinfo, ierr);
}

_EXTERN_C_ void mpi_info_dup(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *ierr) { 
    MPI_Info_dup_fortran_wrapper(info, newinfo, ierr);
}

_EXTERN_C_ void mpi_info_dup_(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *ierr) { 
    MPI_Info_dup_fortran_wrapper(info, newinfo, ierr);
}

_EXTERN_C_ void mpi_info_dup__(MPI_Fint *info, MPI_Fint *newinfo, MPI_Fint *ierr) { 
    MPI_Info_dup_fortran_wrapper(info, newinfo, ierr);
}

/* ================= End Wrappers for MPI_Info_dup ================= */


/* ================== C Wrappers for MPI_Info_free ================== */
_EXTERN_C_ int PMPI_Info_free(MPI_Info *info);
_EXTERN_C_ int MPI_Info_free(MPI_Info *info) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_free(info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_free =============== */
static void MPI_Info_free_fortran_wrapper(MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_free((MPI_Info*)info);
#else /* MPI-2 safe call */
    MPI_Info temp_info;
    temp_info = MPI_Info_f2c(*info);
    _wrap_py_return_val = MPI_Info_free(&temp_info);
    *info = MPI_Info_c2f(temp_info);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_FREE(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_free_fortran_wrapper(info, ierr);
}

_EXTERN_C_ void mpi_info_free(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_free_fortran_wrapper(info, ierr);
}

_EXTERN_C_ void mpi_info_free_(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_free_fortran_wrapper(info, ierr);
}

_EXTERN_C_ void mpi_info_free__(MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Info_free_fortran_wrapper(info, ierr);
}

/* ================= End Wrappers for MPI_Info_free ================= */


/* ================== C Wrappers for MPI_Info_get ================== */
_EXTERN_C_ int PMPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);
_EXTERN_C_ int MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_get(info, key, valuelen, value, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_get =============== */
static void MPI_Info_get_fortran_wrapper(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *value, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_get((MPI_Info)(*info), (const char*)key, *valuelen, (char*)value, (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Info_get(MPI_Info_f2c(*info), (const char*)key, *valuelen, (char*)value, (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_GET(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *value, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_fortran_wrapper(info, key, valuelen, value, flag, ierr);
}

_EXTERN_C_ void mpi_info_get(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *value, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_fortran_wrapper(info, key, valuelen, value, flag, ierr);
}

_EXTERN_C_ void mpi_info_get_(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *value, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_fortran_wrapper(info, key, valuelen, value, flag, ierr);
}

_EXTERN_C_ void mpi_info_get__(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *value, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_fortran_wrapper(info, key, valuelen, value, flag, ierr);
}

/* ================= End Wrappers for MPI_Info_get ================= */


/* ================== C Wrappers for MPI_Info_get_nkeys ================== */
_EXTERN_C_ int PMPI_Info_get_nkeys(MPI_Info info, int *nkeys);
_EXTERN_C_ int MPI_Info_get_nkeys(MPI_Info info, int *nkeys) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_get_nkeys(info, nkeys);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_get_nkeys =============== */
static void MPI_Info_get_nkeys_fortran_wrapper(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_get_nkeys((MPI_Info)(*info), (int*)nkeys);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Info_get_nkeys(MPI_Info_f2c(*info), (int*)nkeys);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_GET_NKEYS(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *ierr) { 
    MPI_Info_get_nkeys_fortran_wrapper(info, nkeys, ierr);
}

_EXTERN_C_ void mpi_info_get_nkeys(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *ierr) { 
    MPI_Info_get_nkeys_fortran_wrapper(info, nkeys, ierr);
}

_EXTERN_C_ void mpi_info_get_nkeys_(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *ierr) { 
    MPI_Info_get_nkeys_fortran_wrapper(info, nkeys, ierr);
}

_EXTERN_C_ void mpi_info_get_nkeys__(MPI_Fint *info, MPI_Fint *nkeys, MPI_Fint *ierr) { 
    MPI_Info_get_nkeys_fortran_wrapper(info, nkeys, ierr);
}

/* ================= End Wrappers for MPI_Info_get_nkeys ================= */


/* ================== C Wrappers for MPI_Info_get_nthkey ================== */
_EXTERN_C_ int PMPI_Info_get_nthkey(MPI_Info info, int n, char *key);
_EXTERN_C_ int MPI_Info_get_nthkey(MPI_Info info, int n, char *key) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_get_nthkey(info, n, key);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_get_nthkey =============== */
static void MPI_Info_get_nthkey_fortran_wrapper(MPI_Fint *info, MPI_Fint *n, MPI_Fint *key, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_get_nthkey((MPI_Info)(*info), *n, (char*)key);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Info_get_nthkey(MPI_Info_f2c(*info), *n, (char*)key);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_GET_NTHKEY(MPI_Fint *info, MPI_Fint *n, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_get_nthkey_fortran_wrapper(info, n, key, ierr);
}

_EXTERN_C_ void mpi_info_get_nthkey(MPI_Fint *info, MPI_Fint *n, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_get_nthkey_fortran_wrapper(info, n, key, ierr);
}

_EXTERN_C_ void mpi_info_get_nthkey_(MPI_Fint *info, MPI_Fint *n, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_get_nthkey_fortran_wrapper(info, n, key, ierr);
}

_EXTERN_C_ void mpi_info_get_nthkey__(MPI_Fint *info, MPI_Fint *n, MPI_Fint *key, MPI_Fint *ierr) { 
    MPI_Info_get_nthkey_fortran_wrapper(info, n, key, ierr);
}

/* ================= End Wrappers for MPI_Info_get_nthkey ================= */


/* ================== C Wrappers for MPI_Info_get_valuelen ================== */
_EXTERN_C_ int PMPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);
_EXTERN_C_ int MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_get_valuelen(info, key, valuelen, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_get_valuelen =============== */
static void MPI_Info_get_valuelen_fortran_wrapper(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_get_valuelen((MPI_Info)(*info), (const char*)key, (int*)valuelen, (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Info_get_valuelen(MPI_Info_f2c(*info), (const char*)key, (int*)valuelen, (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_GET_VALUELEN(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_valuelen_fortran_wrapper(info, key, valuelen, flag, ierr);
}

_EXTERN_C_ void mpi_info_get_valuelen(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_valuelen_fortran_wrapper(info, key, valuelen, flag, ierr);
}

_EXTERN_C_ void mpi_info_get_valuelen_(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_valuelen_fortran_wrapper(info, key, valuelen, flag, ierr);
}

_EXTERN_C_ void mpi_info_get_valuelen__(MPI_Fint *info, MPI_Fint *key, MPI_Fint *valuelen, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Info_get_valuelen_fortran_wrapper(info, key, valuelen, flag, ierr);
}

/* ================= End Wrappers for MPI_Info_get_valuelen ================= */


/* ================== C Wrappers for MPI_Info_set ================== */
_EXTERN_C_ int PMPI_Info_set(MPI_Info info, const char *key, const char *value);
_EXTERN_C_ int MPI_Info_set(MPI_Info info, const char *key, const char *value) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Info_set(info, key, value);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Info_set =============== */
static void MPI_Info_set_fortran_wrapper(MPI_Fint *info, MPI_Fint *key, MPI_Fint *value, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Info_set((MPI_Info)(*info), (const char*)key, (const char*)value);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Info_set(MPI_Info_f2c(*info), (const char*)key, (const char*)value);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INFO_SET(MPI_Fint *info, MPI_Fint *key, MPI_Fint *value, MPI_Fint *ierr) { 
    MPI_Info_set_fortran_wrapper(info, key, value, ierr);
}

_EXTERN_C_ void mpi_info_set(MPI_Fint *info, MPI_Fint *key, MPI_Fint *value, MPI_Fint *ierr) { 
    MPI_Info_set_fortran_wrapper(info, key, value, ierr);
}

_EXTERN_C_ void mpi_info_set_(MPI_Fint *info, MPI_Fint *key, MPI_Fint *value, MPI_Fint *ierr) { 
    MPI_Info_set_fortran_wrapper(info, key, value, ierr);
}

_EXTERN_C_ void mpi_info_set__(MPI_Fint *info, MPI_Fint *key, MPI_Fint *value, MPI_Fint *ierr) { 
    MPI_Info_set_fortran_wrapper(info, key, value, ierr);
}

/* ================= End Wrappers for MPI_Info_set ================= */


/* ================== C Wrappers for MPI_Init_thread ================== */
_EXTERN_C_ int PMPI_Init_thread(int *argc, char ***argv, int required, int *provided);
_EXTERN_C_ int MPI_Init_thread(int *argc, char ***argv, int required, int *provided) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Init_thread(argc, argv, required, provided);
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


/* ================== C Wrappers for MPI_Initialized ================== */
_EXTERN_C_ int PMPI_Initialized(int *flag);
_EXTERN_C_ int MPI_Initialized(int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Initialized(flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Initialized =============== */
static void MPI_Initialized_fortran_wrapper(MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Initialized((int*)flag);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INITIALIZED(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Initialized_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_initialized(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Initialized_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_initialized_(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Initialized_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_initialized__(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Initialized_fortran_wrapper(flag, ierr);
}

/* ================= End Wrappers for MPI_Initialized ================= */


/* ================== C Wrappers for MPI_Intercomm_create ================== */
_EXTERN_C_ int PMPI_Intercomm_create(MPI_Comm local_comm, int local_leader, MPI_Comm bridge_comm, int remote_leader, int tag, MPI_Comm *newintercomm);
_EXTERN_C_ int MPI_Intercomm_create(MPI_Comm local_comm, int local_leader, MPI_Comm bridge_comm, int remote_leader, int tag, MPI_Comm *newintercomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(local_comm);
swap_world(bridge_comm);

   _wrap_py_return_val = PMPI_Intercomm_create(local_comm, local_leader, bridge_comm, remote_leader, tag, newintercomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Intercomm_create =============== */
static void MPI_Intercomm_create_fortran_wrapper(MPI_Fint *local_comm, MPI_Fint *local_leader, MPI_Fint *bridge_comm, MPI_Fint *remote_leader, MPI_Fint *tag, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Intercomm_create((MPI_Comm)(*local_comm), *local_leader, (MPI_Comm)(*bridge_comm), *remote_leader, *tag, (MPI_Comm*)newintercomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newintercomm;
    temp_newintercomm = MPI_Comm_f2c(*newintercomm);
    _wrap_py_return_val = MPI_Intercomm_create(MPI_Comm_f2c(*local_comm), *local_leader, MPI_Comm_f2c(*bridge_comm), *remote_leader, *tag, &temp_newintercomm);
    *newintercomm = MPI_Comm_c2f(temp_newintercomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INTERCOMM_CREATE(MPI_Fint *local_comm, MPI_Fint *local_leader, MPI_Fint *bridge_comm, MPI_Fint *remote_leader, MPI_Fint *tag, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_create_fortran_wrapper(local_comm, local_leader, bridge_comm, remote_leader, tag, newintercomm, ierr);
}

_EXTERN_C_ void mpi_intercomm_create(MPI_Fint *local_comm, MPI_Fint *local_leader, MPI_Fint *bridge_comm, MPI_Fint *remote_leader, MPI_Fint *tag, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_create_fortran_wrapper(local_comm, local_leader, bridge_comm, remote_leader, tag, newintercomm, ierr);
}

_EXTERN_C_ void mpi_intercomm_create_(MPI_Fint *local_comm, MPI_Fint *local_leader, MPI_Fint *bridge_comm, MPI_Fint *remote_leader, MPI_Fint *tag, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_create_fortran_wrapper(local_comm, local_leader, bridge_comm, remote_leader, tag, newintercomm, ierr);
}

_EXTERN_C_ void mpi_intercomm_create__(MPI_Fint *local_comm, MPI_Fint *local_leader, MPI_Fint *bridge_comm, MPI_Fint *remote_leader, MPI_Fint *tag, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_create_fortran_wrapper(local_comm, local_leader, bridge_comm, remote_leader, tag, newintercomm, ierr);
}

/* ================= End Wrappers for MPI_Intercomm_create ================= */


/* ================== C Wrappers for MPI_Intercomm_merge ================== */
_EXTERN_C_ int PMPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintercomm);
_EXTERN_C_ int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintercomm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(intercomm);

   _wrap_py_return_val = PMPI_Intercomm_merge(intercomm, high, newintercomm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Intercomm_merge =============== */
static void MPI_Intercomm_merge_fortran_wrapper(MPI_Fint *intercomm, MPI_Fint *high, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Intercomm_merge((MPI_Comm)(*intercomm), *high, (MPI_Comm*)newintercomm);
#else /* MPI-2 safe call */
    MPI_Comm temp_newintercomm;
    temp_newintercomm = MPI_Comm_f2c(*newintercomm);
    _wrap_py_return_val = MPI_Intercomm_merge(MPI_Comm_f2c(*intercomm), *high, &temp_newintercomm);
    *newintercomm = MPI_Comm_c2f(temp_newintercomm);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INTERCOMM_MERGE(MPI_Fint *intercomm, MPI_Fint *high, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_merge_fortran_wrapper(intercomm, high, newintercomm, ierr);
}

_EXTERN_C_ void mpi_intercomm_merge(MPI_Fint *intercomm, MPI_Fint *high, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_merge_fortran_wrapper(intercomm, high, newintercomm, ierr);
}

_EXTERN_C_ void mpi_intercomm_merge_(MPI_Fint *intercomm, MPI_Fint *high, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_merge_fortran_wrapper(intercomm, high, newintercomm, ierr);
}

_EXTERN_C_ void mpi_intercomm_merge__(MPI_Fint *intercomm, MPI_Fint *high, MPI_Fint *newintercomm, MPI_Fint *ierr) { 
    MPI_Intercomm_merge_fortran_wrapper(intercomm, high, newintercomm, ierr);
}

/* ================= End Wrappers for MPI_Intercomm_merge ================= */


/* ================== C Wrappers for MPI_Iprobe ================== */
_EXTERN_C_ int PMPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status);
_EXTERN_C_ int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iprobe(source, tag, comm, flag, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iprobe =============== */
static void MPI_Iprobe_fortran_wrapper(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iprobe(*source, *tag, (MPI_Comm)(*comm), (int*)flag, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Iprobe(*source, *tag, MPI_Comm_f2c(*comm), (int*)flag, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IPROBE(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Iprobe_fortran_wrapper(source, tag, comm, flag, status, ierr);
}

_EXTERN_C_ void mpi_iprobe(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Iprobe_fortran_wrapper(source, tag, comm, flag, status, ierr);
}

_EXTERN_C_ void mpi_iprobe_(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Iprobe_fortran_wrapper(source, tag, comm, flag, status, ierr);
}

_EXTERN_C_ void mpi_iprobe__(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Iprobe_fortran_wrapper(source, tag, comm, flag, status, ierr);
}

/* ================= End Wrappers for MPI_Iprobe ================= */


/* ================== C Wrappers for MPI_Irecv ================== */
_EXTERN_C_ int PMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Irecv(buf, count, datatype, source, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Irecv =============== */
static void MPI_Irecv_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Irecv((void*)buf, *count, (MPI_Datatype)(*datatype), *source, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Irecv((void*)buf, *count, MPI_Type_f2c(*datatype), *source, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IRECV(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irecv_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_irecv(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irecv_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_irecv_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irecv_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_irecv__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irecv_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Irecv ================= */


/* ================== C Wrappers for MPI_Ireduce ================== */
_EXTERN_C_ int PMPI_Ireduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ireduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ireduce(sendbuf, recvbuf, count, datatype, op, root, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ireduce =============== */
static void MPI_Ireduce_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ireduce((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), *root, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ireduce((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), *root, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IREDUCE(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ireduce ================= */


/* ================== C Wrappers for MPI_Ireduce_scatter ================== */
_EXTERN_C_ int PMPI_Ireduce_scatter(const void *sendbuf, void *recvbuf, const int recvcounts[], MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ireduce_scatter(const void *sendbuf, void *recvbuf, const int recvcounts[], MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ireduce_scatter(sendbuf, recvbuf, recvcounts, datatype, op, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ireduce_scatter =============== */
static void MPI_Ireduce_scatter_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ireduce_scatter((const void*)sendbuf, (void*)recvbuf, (const int*)recvcounts, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ireduce_scatter((const void*)sendbuf, (void*)recvbuf, (const int*)recvcounts, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IREDUCE_SCATTER(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_scatter(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_scatter_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_scatter__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ireduce_scatter ================= */


/* ================== C Wrappers for MPI_Ireduce_scatter_block ================== */
_EXTERN_C_ int PMPI_Ireduce_scatter_block(const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ireduce_scatter_block(const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ireduce_scatter_block(sendbuf, recvbuf, recvcount, datatype, op, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ireduce_scatter_block =============== */
static void MPI_Ireduce_scatter_block_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ireduce_scatter_block((const void*)sendbuf, (void*)recvbuf, *recvcount, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ireduce_scatter_block((const void*)sendbuf, (void*)recvbuf, *recvcount, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IREDUCE_SCATTER_BLOCK(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_scatter_block(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_scatter_block_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_ireduce_scatter_block__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ireduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ireduce_scatter_block ================= */


/* ================== C Wrappers for MPI_Irsend ================== */
_EXTERN_C_ int PMPI_Irsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Irsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Irsend(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Irsend =============== */
static void MPI_Irsend_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Irsend((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Irsend((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IRSEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_irsend(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_irsend_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_irsend__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Irsend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Irsend ================= */


/* ================== C Wrappers for MPI_Is_thread_main ================== */
_EXTERN_C_ int PMPI_Is_thread_main(int *flag);
_EXTERN_C_ int MPI_Is_thread_main(int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Is_thread_main(flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Is_thread_main =============== */
static void MPI_Is_thread_main_fortran_wrapper(MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Is_thread_main((int*)flag);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_IS_THREAD_MAIN(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Is_thread_main_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_is_thread_main(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Is_thread_main_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_is_thread_main_(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Is_thread_main_fortran_wrapper(flag, ierr);
}

_EXTERN_C_ void mpi_is_thread_main__(MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Is_thread_main_fortran_wrapper(flag, ierr);
}

/* ================= End Wrappers for MPI_Is_thread_main ================= */


/* ================== C Wrappers for MPI_Iscan ================== */
_EXTERN_C_ int PMPI_Iscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iscan(sendbuf, recvbuf, count, datatype, op, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iscan =============== */
static void MPI_Iscan_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iscan((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iscan((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ISCAN(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscan(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscan_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscan__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iscan ================= */


/* ================== C Wrappers for MPI_Iscatter ================== */
_EXTERN_C_ int PMPI_Iscatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iscatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iscatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iscatter =============== */
static void MPI_Iscatter_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iscatter((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iscatter((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ISCATTER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscatter(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscatter_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscatter__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iscatter ================= */


/* ================== C Wrappers for MPI_Iscatterv ================== */
_EXTERN_C_ int PMPI_Iscatterv(const void *sendbuf, const int sendcounts[], const int displs[], MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Iscatterv(const void *sendbuf, const int sendcounts[], const int displs[], MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Iscatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Iscatterv =============== */
static void MPI_Iscatterv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Iscatterv((const void*)sendbuf, (const int*)sendcounts, (const int*)displs, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Iscatterv((const void*)sendbuf, (const int*)sendcounts, (const int*)displs, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ISCATTERV(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscatterv(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscatterv_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

_EXTERN_C_ void mpi_iscatterv__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Iscatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Iscatterv ================= */


/* ================== C Wrappers for MPI_Isend ================== */
_EXTERN_C_ int PMPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Isend =============== */
static void MPI_Isend_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Isend((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Isend((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ISEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Isend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_isend(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Isend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_isend_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Isend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_isend__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Isend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Isend ================= */


/* ================== C Wrappers for MPI_Issend ================== */
_EXTERN_C_ int PMPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Issend(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Issend =============== */
static void MPI_Issend_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Issend((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Issend((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ISSEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Issend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_issend(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Issend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_issend_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Issend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_issend__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Issend_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Issend ================= */


/* ================== C Wrappers for MPI_Keyval_create ================== */
_EXTERN_C_ int PMPI_Keyval_create(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, int *keyval, void *extra_state);
_EXTERN_C_ int MPI_Keyval_create(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, int *keyval, void *extra_state) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Keyval_create(copy_fn, delete_fn, keyval, extra_state);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Keyval_create =============== */
static void MPI_Keyval_create_fortran_wrapper(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, MPI_Fint *keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Keyval_create((MPI_Copy_function*)copy_fn, (MPI_Delete_function*)delete_fn, (int*)keyval, (void*)extra_state);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_KEYVAL_CREATE(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, MPI_Fint *keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Keyval_create_fortran_wrapper(copy_fn, delete_fn, keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_keyval_create(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, MPI_Fint *keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Keyval_create_fortran_wrapper(copy_fn, delete_fn, keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_keyval_create_(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, MPI_Fint *keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Keyval_create_fortran_wrapper(copy_fn, delete_fn, keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_keyval_create__(MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn, MPI_Fint *keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Keyval_create_fortran_wrapper(copy_fn, delete_fn, keyval, extra_state, ierr);
}

/* ================= End Wrappers for MPI_Keyval_create ================= */


/* ================== C Wrappers for MPI_Keyval_free ================== */
_EXTERN_C_ int PMPI_Keyval_free(int *keyval);
_EXTERN_C_ int MPI_Keyval_free(int *keyval) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Keyval_free(keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Keyval_free =============== */
static void MPI_Keyval_free_fortran_wrapper(MPI_Fint *keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Keyval_free((int*)keyval);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_KEYVAL_FREE(MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Keyval_free_fortran_wrapper(keyval, ierr);
}

_EXTERN_C_ void mpi_keyval_free(MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Keyval_free_fortran_wrapper(keyval, ierr);
}

_EXTERN_C_ void mpi_keyval_free_(MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Keyval_free_fortran_wrapper(keyval, ierr);
}

_EXTERN_C_ void mpi_keyval_free__(MPI_Fint *keyval, MPI_Fint *ierr) { 
    MPI_Keyval_free_fortran_wrapper(keyval, ierr);
}

/* ================= End Wrappers for MPI_Keyval_free ================= */


/* ================== C Wrappers for MPI_Lookup_name ================== */
_EXTERN_C_ int PMPI_Lookup_name(const char *service_name, MPI_Info info, char *port_name);
_EXTERN_C_ int MPI_Lookup_name(const char *service_name, MPI_Info info, char *port_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Lookup_name(service_name, info, port_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Lookup_name =============== */
static void MPI_Lookup_name_fortran_wrapper(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Lookup_name((const char*)service_name, (MPI_Info)(*info), (char*)port_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Lookup_name((const char*)service_name, MPI_Info_f2c(*info), (char*)port_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_LOOKUP_NAME(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Lookup_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_lookup_name(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Lookup_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_lookup_name_(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Lookup_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_lookup_name__(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Lookup_name_fortran_wrapper(service_name, info, port_name, ierr);
}

/* ================= End Wrappers for MPI_Lookup_name ================= */


/* ================== C Wrappers for MPI_Mprobe ================== */
_EXTERN_C_ int PMPI_Mprobe(int source, int tag, MPI_Comm comm, MPI_Message *message, MPI_Status *status);
_EXTERN_C_ int MPI_Mprobe(int source, int tag, MPI_Comm comm, MPI_Message *message, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Mprobe(source, tag, comm, message, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Mprobe =============== */
static void MPI_Mprobe_fortran_wrapper(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Mprobe(*source, *tag, (MPI_Comm)(*comm), (MPI_Message*)message, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Mprobe(*source, *tag, MPI_Comm_f2c(*comm), (MPI_Message*)message, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_MPROBE(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mprobe_fortran_wrapper(source, tag, comm, message, status, ierr);
}

_EXTERN_C_ void mpi_mprobe(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mprobe_fortran_wrapper(source, tag, comm, message, status, ierr);
}

_EXTERN_C_ void mpi_mprobe_(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mprobe_fortran_wrapper(source, tag, comm, message, status, ierr);
}

_EXTERN_C_ void mpi_mprobe__(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mprobe_fortran_wrapper(source, tag, comm, message, status, ierr);
}

/* ================= End Wrappers for MPI_Mprobe ================= */


/* ================== C Wrappers for MPI_Mrecv ================== */
_EXTERN_C_ int PMPI_Mrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Status *status);
_EXTERN_C_ int MPI_Mrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Mrecv(buf, count, type, message, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Mrecv =============== */
static void MPI_Mrecv_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Mrecv((void*)buf, *count, (MPI_Datatype)(*type), (MPI_Message*)message, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Mrecv((void*)buf, *count, MPI_Type_f2c(*type), (MPI_Message*)message, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_MRECV(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mrecv_fortran_wrapper(buf, count, type, message, status, ierr);
}

_EXTERN_C_ void mpi_mrecv(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mrecv_fortran_wrapper(buf, count, type, message, status, ierr);
}

_EXTERN_C_ void mpi_mrecv_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mrecv_fortran_wrapper(buf, count, type, message, status, ierr);
}

_EXTERN_C_ void mpi_mrecv__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *type, MPI_Fint *message, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Mrecv_fortran_wrapper(buf, count, type, message, status, ierr);
}

/* ================= End Wrappers for MPI_Mrecv ================= */


/* ================== C Wrappers for MPI_Neighbor_allgather ================== */
_EXTERN_C_ int PMPI_Neighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Neighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Neighbor_allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Neighbor_allgather =============== */
static void MPI_Neighbor_allgather_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Neighbor_allgather((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Neighbor_allgather((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_NEIGHBOR_ALLGATHER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_allgather(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_allgather_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_allgather__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgather_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Neighbor_allgather ================= */


/* ================== C Wrappers for MPI_Neighbor_allgatherv ================== */
_EXTERN_C_ int PMPI_Neighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Neighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Neighbor_allgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Neighbor_allgatherv =============== */
static void MPI_Neighbor_allgatherv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Neighbor_allgatherv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Neighbor_allgatherv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)displs, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_NEIGHBOR_ALLGATHERV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_allgatherv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_allgatherv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_allgatherv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint displs[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_allgatherv_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Neighbor_allgatherv ================= */


/* ================== C Wrappers for MPI_Neighbor_alltoall ================== */
_EXTERN_C_ int PMPI_Neighbor_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Neighbor_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Neighbor_alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Neighbor_alltoall =============== */
static void MPI_Neighbor_alltoall_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Neighbor_alltoall((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Neighbor_alltoall((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_NEIGHBOR_ALLTOALL(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoall(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoall_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoall__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Neighbor_alltoall ================= */


/* ================== C Wrappers for MPI_Neighbor_alltoallv ================== */
_EXTERN_C_ int PMPI_Neighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm);
_EXTERN_C_ int MPI_Neighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[], MPI_Datatype sendtype, void *recvbuf, const int recvcounts[], const int rdispls[], MPI_Datatype recvtype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Neighbor_alltoallv(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Neighbor_alltoallv =============== */
static void MPI_Neighbor_alltoallv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Neighbor_alltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, (MPI_Datatype)(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Neighbor_alltoallv((const void*)sendbuf, (const int*)sendcounts, (const int*)sdispls, MPI_Type_f2c(*sendtype), (void*)recvbuf, (const int*)recvcounts, (const int*)rdispls, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_NEIGHBOR_ALLTOALLV(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoallv(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoallv_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoallv__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Neighbor_alltoallv ================= */


/* ================== C Wrappers for MPI_Neighbor_alltoallw ================== */
_EXTERN_C_ int PMPI_Neighbor_alltoallw(const void *sendbuf, const int sendcounts[], const MPI_Aint sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const MPI_Aint rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm);
_EXTERN_C_ int MPI_Neighbor_alltoallw(const void *sendbuf, const int sendcounts[], const MPI_Aint sdispls[], const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[], const MPI_Aint rdispls[], const MPI_Datatype recvtypes[], MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Neighbor_alltoallw(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Neighbor_alltoallw =============== */
static void MPI_Neighbor_alltoallw_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Neighbor_alltoallw((const void*)sendbuf, (const int*)sendcounts, (const MPI_Aint*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const MPI_Aint*)rdispls, (const MPI_Datatype*)recvtypes, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Neighbor_alltoallw((const void*)sendbuf, (const int*)sendcounts, (const MPI_Aint*)sdispls, (const MPI_Datatype*)sendtypes, (void*)recvbuf, (const int*)recvcounts, (const MPI_Aint*)rdispls, (const MPI_Datatype*)recvtypes, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_NEIGHBOR_ALLTOALLW(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoallw(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoallw_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_neighbor_alltoallw__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Neighbor_alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

/* ================= End Wrappers for MPI_Neighbor_alltoallw ================= */


/* ================== C Wrappers for MPI_Op_commutative ================== */
_EXTERN_C_ int PMPI_Op_commutative(MPI_Op op, int *commute);
_EXTERN_C_ int MPI_Op_commutative(MPI_Op op, int *commute) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Op_commutative(op, commute);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Op_commutative =============== */
static void MPI_Op_commutative_fortran_wrapper(MPI_Fint *op, MPI_Fint *commute, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Op_commutative((MPI_Op)(*op), (int*)commute);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Op_commutative(MPI_Op_f2c(*op), (int*)commute);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_OP_COMMUTATIVE(MPI_Fint *op, MPI_Fint *commute, MPI_Fint *ierr) { 
    MPI_Op_commutative_fortran_wrapper(op, commute, ierr);
}

_EXTERN_C_ void mpi_op_commutative(MPI_Fint *op, MPI_Fint *commute, MPI_Fint *ierr) { 
    MPI_Op_commutative_fortran_wrapper(op, commute, ierr);
}

_EXTERN_C_ void mpi_op_commutative_(MPI_Fint *op, MPI_Fint *commute, MPI_Fint *ierr) { 
    MPI_Op_commutative_fortran_wrapper(op, commute, ierr);
}

_EXTERN_C_ void mpi_op_commutative__(MPI_Fint *op, MPI_Fint *commute, MPI_Fint *ierr) { 
    MPI_Op_commutative_fortran_wrapper(op, commute, ierr);
}

/* ================= End Wrappers for MPI_Op_commutative ================= */


/* ================== C Wrappers for MPI_Op_create ================== */
_EXTERN_C_ int PMPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op);
_EXTERN_C_ int MPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Op_create(function, commute, op);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Op_create =============== */
static void MPI_Op_create_fortran_wrapper(MPI_User_function *function, MPI_Fint *commute, MPI_Fint *op, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Op_create((MPI_User_function*)function, *commute, (MPI_Op*)op);
#else /* MPI-2 safe call */
    MPI_Op temp_op;
    temp_op = MPI_Op_f2c(*op);
    _wrap_py_return_val = MPI_Op_create((MPI_User_function*)function, *commute, &temp_op);
    *op = MPI_Op_c2f(temp_op);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_OP_CREATE(MPI_User_function *function, MPI_Fint *commute, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_create_fortran_wrapper(function, commute, op, ierr);
}

_EXTERN_C_ void mpi_op_create(MPI_User_function *function, MPI_Fint *commute, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_create_fortran_wrapper(function, commute, op, ierr);
}

_EXTERN_C_ void mpi_op_create_(MPI_User_function *function, MPI_Fint *commute, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_create_fortran_wrapper(function, commute, op, ierr);
}

_EXTERN_C_ void mpi_op_create__(MPI_User_function *function, MPI_Fint *commute, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_create_fortran_wrapper(function, commute, op, ierr);
}

/* ================= End Wrappers for MPI_Op_create ================= */


/* ================== C Wrappers for MPI_Op_free ================== */
_EXTERN_C_ int PMPI_Op_free(MPI_Op *op);
_EXTERN_C_ int MPI_Op_free(MPI_Op *op) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Op_free(op);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Op_free =============== */
static void MPI_Op_free_fortran_wrapper(MPI_Fint *op, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Op_free((MPI_Op*)op);
#else /* MPI-2 safe call */
    MPI_Op temp_op;
    temp_op = MPI_Op_f2c(*op);
    _wrap_py_return_val = MPI_Op_free(&temp_op);
    *op = MPI_Op_c2f(temp_op);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_OP_FREE(MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_free_fortran_wrapper(op, ierr);
}

_EXTERN_C_ void mpi_op_free(MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_free_fortran_wrapper(op, ierr);
}

_EXTERN_C_ void mpi_op_free_(MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_free_fortran_wrapper(op, ierr);
}

_EXTERN_C_ void mpi_op_free__(MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Op_free_fortran_wrapper(op, ierr);
}

/* ================= End Wrappers for MPI_Op_free ================= */


/* ================== C Wrappers for MPI_Open_port ================== */
_EXTERN_C_ int PMPI_Open_port(MPI_Info info, char *port_name);
_EXTERN_C_ int MPI_Open_port(MPI_Info info, char *port_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Open_port(info, port_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Open_port =============== */
static void MPI_Open_port_fortran_wrapper(MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Open_port((MPI_Info)(*info), (char*)port_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Open_port(MPI_Info_f2c(*info), (char*)port_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_OPEN_PORT(MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Open_port_fortran_wrapper(info, port_name, ierr);
}

_EXTERN_C_ void mpi_open_port(MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Open_port_fortran_wrapper(info, port_name, ierr);
}

_EXTERN_C_ void mpi_open_port_(MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Open_port_fortran_wrapper(info, port_name, ierr);
}

_EXTERN_C_ void mpi_open_port__(MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Open_port_fortran_wrapper(info, port_name, ierr);
}

/* ================= End Wrappers for MPI_Open_port ================= */


/* ================== C Wrappers for MPI_Pack ================== */
_EXTERN_C_ int PMPI_Pack(const void *inbuf, int incount, MPI_Datatype datatype, void *outbuf, int outsize, int *position, MPI_Comm comm);
_EXTERN_C_ int MPI_Pack(const void *inbuf, int incount, MPI_Datatype datatype, void *outbuf, int outsize, int *position, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Pack(inbuf, incount, datatype, outbuf, outsize, position, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Pack =============== */
static void MPI_Pack_fortran_wrapper(MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Fint *outsize, MPI_Fint *position, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Pack((const void*)inbuf, *incount, (MPI_Datatype)(*datatype), (void*)outbuf, *outsize, (int*)position, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Pack((const void*)inbuf, *incount, MPI_Type_f2c(*datatype), (void*)outbuf, *outsize, (int*)position, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PACK(MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Fint *outsize, MPI_Fint *position, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Pack_fortran_wrapper(inbuf, incount, datatype, outbuf, outsize, position, comm, ierr);
}

_EXTERN_C_ void mpi_pack(MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Fint *outsize, MPI_Fint *position, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Pack_fortran_wrapper(inbuf, incount, datatype, outbuf, outsize, position, comm, ierr);
}

_EXTERN_C_ void mpi_pack_(MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Fint *outsize, MPI_Fint *position, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Pack_fortran_wrapper(inbuf, incount, datatype, outbuf, outsize, position, comm, ierr);
}

_EXTERN_C_ void mpi_pack__(MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Fint *outsize, MPI_Fint *position, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Pack_fortran_wrapper(inbuf, incount, datatype, outbuf, outsize, position, comm, ierr);
}

/* ================= End Wrappers for MPI_Pack ================= */


/* ================== C Wrappers for MPI_Pack_external ================== */
_EXTERN_C_ int PMPI_Pack_external(const char datarep[], const void *inbuf, int incount, MPI_Datatype datatype, void *outbuf, MPI_Aint outsize, MPI_Aint *position);
_EXTERN_C_ int MPI_Pack_external(const char datarep[], const void *inbuf, int incount, MPI_Datatype datatype, void *outbuf, MPI_Aint outsize, MPI_Aint *position) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Pack_external(datarep, inbuf, incount, datatype, outbuf, outsize, position);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Pack_external =============== */
static void MPI_Pack_external_fortran_wrapper(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Aint *outsize, MPI_Aint *position, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Pack_external((const char*)datarep, (const void*)inbuf, *incount, (MPI_Datatype)(*datatype), (void*)outbuf, *outsize, (MPI_Aint*)position);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Pack_external((const char*)datarep, (const void*)inbuf, *incount, MPI_Type_f2c(*datatype), (void*)outbuf, *outsize, (MPI_Aint*)position);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PACK_EXTERNAL(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Aint *outsize, MPI_Aint *position, MPI_Fint *ierr) { 
    MPI_Pack_external_fortran_wrapper(datarep, inbuf, incount, datatype, outbuf, outsize, position, ierr);
}

_EXTERN_C_ void mpi_pack_external(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Aint *outsize, MPI_Aint *position, MPI_Fint *ierr) { 
    MPI_Pack_external_fortran_wrapper(datarep, inbuf, incount, datatype, outbuf, outsize, position, ierr);
}

_EXTERN_C_ void mpi_pack_external_(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Aint *outsize, MPI_Aint *position, MPI_Fint *ierr) { 
    MPI_Pack_external_fortran_wrapper(datarep, inbuf, incount, datatype, outbuf, outsize, position, ierr);
}

_EXTERN_C_ void mpi_pack_external__(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *outbuf, MPI_Aint *outsize, MPI_Aint *position, MPI_Fint *ierr) { 
    MPI_Pack_external_fortran_wrapper(datarep, inbuf, incount, datatype, outbuf, outsize, position, ierr);
}

/* ================= End Wrappers for MPI_Pack_external ================= */


/* ================== C Wrappers for MPI_Pack_external_size ================== */
_EXTERN_C_ int PMPI_Pack_external_size(const char datarep[], int incount, MPI_Datatype datatype, MPI_Aint *size);
_EXTERN_C_ int MPI_Pack_external_size(const char datarep[], int incount, MPI_Datatype datatype, MPI_Aint *size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Pack_external_size(datarep, incount, datatype, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Pack_external_size =============== */
static void MPI_Pack_external_size_fortran_wrapper(MPI_Fint datarep[], MPI_Fint *incount, MPI_Fint *datatype, MPI_Aint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Pack_external_size((const char*)datarep, *incount, (MPI_Datatype)(*datatype), (MPI_Aint*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Pack_external_size((const char*)datarep, *incount, MPI_Type_f2c(*datatype), (MPI_Aint*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PACK_EXTERNAL_SIZE(MPI_Fint datarep[], MPI_Fint *incount, MPI_Fint *datatype, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Pack_external_size_fortran_wrapper(datarep, incount, datatype, size, ierr);
}

_EXTERN_C_ void mpi_pack_external_size(MPI_Fint datarep[], MPI_Fint *incount, MPI_Fint *datatype, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Pack_external_size_fortran_wrapper(datarep, incount, datatype, size, ierr);
}

_EXTERN_C_ void mpi_pack_external_size_(MPI_Fint datarep[], MPI_Fint *incount, MPI_Fint *datatype, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Pack_external_size_fortran_wrapper(datarep, incount, datatype, size, ierr);
}

_EXTERN_C_ void mpi_pack_external_size__(MPI_Fint datarep[], MPI_Fint *incount, MPI_Fint *datatype, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Pack_external_size_fortran_wrapper(datarep, incount, datatype, size, ierr);
}

/* ================= End Wrappers for MPI_Pack_external_size ================= */


/* ================== C Wrappers for MPI_Pack_size ================== */
_EXTERN_C_ int PMPI_Pack_size(int incount, MPI_Datatype datatype, MPI_Comm comm, int *size);
_EXTERN_C_ int MPI_Pack_size(int incount, MPI_Datatype datatype, MPI_Comm comm, int *size) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Pack_size(incount, datatype, comm, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Pack_size =============== */
static void MPI_Pack_size_fortran_wrapper(MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Pack_size(*incount, (MPI_Datatype)(*datatype), (MPI_Comm)(*comm), (int*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Pack_size(*incount, MPI_Type_f2c(*datatype), MPI_Comm_f2c(*comm), (int*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PACK_SIZE(MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Pack_size_fortran_wrapper(incount, datatype, comm, size, ierr);
}

_EXTERN_C_ void mpi_pack_size(MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Pack_size_fortran_wrapper(incount, datatype, comm, size, ierr);
}

_EXTERN_C_ void mpi_pack_size_(MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Pack_size_fortran_wrapper(incount, datatype, comm, size, ierr);
}

_EXTERN_C_ void mpi_pack_size__(MPI_Fint *incount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Pack_size_fortran_wrapper(incount, datatype, comm, size, ierr);
}

/* ================= End Wrappers for MPI_Pack_size ================= */


/* ================== C Wrappers for MPI_Pcontrol ================== */
_EXTERN_C_ int PMPI_Pcontrol(const int level, ...);
_EXTERN_C_ int MPI_Pcontrol(const int level, ...) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Pcontrol(level);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Pcontrol =============== */
static void MPI_Pcontrol_fortran_wrapper(MPI_Fint *level, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Pcontrol(*level);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PCONTROL(MPI_Fint *level, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(level, ierr);
}

_EXTERN_C_ void mpi_pcontrol(MPI_Fint *level, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(level, ierr);
}

_EXTERN_C_ void mpi_pcontrol_(MPI_Fint *level, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(level, ierr);
}

_EXTERN_C_ void mpi_pcontrol__(MPI_Fint *level, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(level, ierr);
}

/* ================= End Wrappers for MPI_Pcontrol ================= */


/* ================== C Wrappers for MPI_Probe ================== */
_EXTERN_C_ int PMPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
_EXTERN_C_ int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Probe(source, tag, comm, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Probe =============== */
static void MPI_Probe_fortran_wrapper(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Probe(*source, *tag, (MPI_Comm)(*comm), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Probe(*source, *tag, MPI_Comm_f2c(*comm), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PROBE(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Probe_fortran_wrapper(source, tag, comm, status, ierr);
}

_EXTERN_C_ void mpi_probe(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Probe_fortran_wrapper(source, tag, comm, status, ierr);
}

_EXTERN_C_ void mpi_probe_(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Probe_fortran_wrapper(source, tag, comm, status, ierr);
}

_EXTERN_C_ void mpi_probe__(MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Probe_fortran_wrapper(source, tag, comm, status, ierr);
}

/* ================= End Wrappers for MPI_Probe ================= */


/* ================== C Wrappers for MPI_Publish_name ================== */
_EXTERN_C_ int PMPI_Publish_name(const char *service_name, MPI_Info info, const char *port_name);
_EXTERN_C_ int MPI_Publish_name(const char *service_name, MPI_Info info, const char *port_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Publish_name(service_name, info, port_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Publish_name =============== */
static void MPI_Publish_name_fortran_wrapper(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Publish_name((const char*)service_name, (MPI_Info)(*info), (const char*)port_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Publish_name((const char*)service_name, MPI_Info_f2c(*info), (const char*)port_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PUBLISH_NAME(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Publish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_publish_name(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Publish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_publish_name_(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Publish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_publish_name__(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Publish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

/* ================= End Wrappers for MPI_Publish_name ================= */


/* ================== C Wrappers for MPI_Put ================== */
_EXTERN_C_ int PMPI_Put(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win);
_EXTERN_C_ int MPI_Put(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Put(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Put =============== */
static void MPI_Put_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Put((const void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Put((const void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_PUT(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Put_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

_EXTERN_C_ void mpi_put(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Put_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

_EXTERN_C_ void mpi_put_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Put_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

_EXTERN_C_ void mpi_put__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Put_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, ierr);
}

/* ================= End Wrappers for MPI_Put ================= */


/* ================== C Wrappers for MPI_Query_thread ================== */
_EXTERN_C_ int PMPI_Query_thread(int *provided);
_EXTERN_C_ int MPI_Query_thread(int *provided) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Query_thread(provided);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Query_thread =============== */
static void MPI_Query_thread_fortran_wrapper(MPI_Fint *provided, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Query_thread((int*)provided);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_QUERY_THREAD(MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Query_thread_fortran_wrapper(provided, ierr);
}

_EXTERN_C_ void mpi_query_thread(MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Query_thread_fortran_wrapper(provided, ierr);
}

_EXTERN_C_ void mpi_query_thread_(MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Query_thread_fortran_wrapper(provided, ierr);
}

_EXTERN_C_ void mpi_query_thread__(MPI_Fint *provided, MPI_Fint *ierr) { 
    MPI_Query_thread_fortran_wrapper(provided, ierr);
}

/* ================= End Wrappers for MPI_Query_thread ================= */


/* ================== C Wrappers for MPI_Raccumulate ================== */
_EXTERN_C_ int PMPI_Raccumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win, MPI_Request *request);
_EXTERN_C_ int MPI_Raccumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Raccumulate(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Raccumulate =============== */
static void MPI_Raccumulate_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Raccumulate((const void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Op)(*op), (MPI_Win)(*win), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Raccumulate((const void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Op_f2c(*op), MPI_Win_f2c(*win), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RACCUMULATE(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Raccumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

_EXTERN_C_ void mpi_raccumulate(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Raccumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

_EXTERN_C_ void mpi_raccumulate_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Raccumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

_EXTERN_C_ void mpi_raccumulate__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Raccumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

/* ================= End Wrappers for MPI_Raccumulate ================= */


/* ================== C Wrappers for MPI_Recv ================== */
_EXTERN_C_ int PMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
_EXTERN_C_ int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Recv =============== */
static void MPI_Recv_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Recv((void*)buf, *count, (MPI_Datatype)(*datatype), *source, *tag, (MPI_Comm)(*comm), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Recv((void*)buf, *count, MPI_Type_f2c(*datatype), *source, *tag, MPI_Comm_f2c(*comm), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RECV(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Recv_fortran_wrapper(buf, count, datatype, source, tag, comm, status, ierr);
}

_EXTERN_C_ void mpi_recv(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Recv_fortran_wrapper(buf, count, datatype, source, tag, comm, status, ierr);
}

_EXTERN_C_ void mpi_recv_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Recv_fortran_wrapper(buf, count, datatype, source, tag, comm, status, ierr);
}

_EXTERN_C_ void mpi_recv__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Recv_fortran_wrapper(buf, count, datatype, source, tag, comm, status, ierr);
}

/* ================= End Wrappers for MPI_Recv ================= */


/* ================== C Wrappers for MPI_Recv_init ================== */
_EXTERN_C_ int PMPI_Recv_init(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Recv_init(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Recv_init(buf, count, datatype, source, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Recv_init =============== */
static void MPI_Recv_init_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Recv_init((void*)buf, *count, (MPI_Datatype)(*datatype), *source, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Recv_init((void*)buf, *count, MPI_Type_f2c(*datatype), *source, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RECV_INIT(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Recv_init_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_recv_init(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Recv_init_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_recv_init_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Recv_init_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_recv_init__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Recv_init_fortran_wrapper(buf, count, datatype, source, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Recv_init ================= */


/* ================== C Wrappers for MPI_Reduce ================== */
_EXTERN_C_ int PMPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
_EXTERN_C_ int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Reduce =============== */
static void MPI_Reduce_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Reduce((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), *root, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Reduce((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), *root, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REDUCE(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, ierr);
}

_EXTERN_C_ void mpi_reduce(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, ierr);
}

_EXTERN_C_ void mpi_reduce__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, root, comm, ierr);
}

/* ================= End Wrappers for MPI_Reduce ================= */


/* ================== C Wrappers for MPI_Reduce_local ================== */
_EXTERN_C_ int PMPI_Reduce_local(const void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype, MPI_Op op);
_EXTERN_C_ int MPI_Reduce_local(const void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype, MPI_Op op) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Reduce_local(inbuf, inoutbuf, count, datatype, op);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Reduce_local =============== */
static void MPI_Reduce_local_fortran_wrapper(MPI_Fint *inbuf, MPI_Fint *inoutbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Reduce_local((const void*)inbuf, (void*)inoutbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Reduce_local((const void*)inbuf, (void*)inoutbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REDUCE_LOCAL(MPI_Fint *inbuf, MPI_Fint *inoutbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Reduce_local_fortran_wrapper(inbuf, inoutbuf, count, datatype, op, ierr);
}

_EXTERN_C_ void mpi_reduce_local(MPI_Fint *inbuf, MPI_Fint *inoutbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Reduce_local_fortran_wrapper(inbuf, inoutbuf, count, datatype, op, ierr);
}

_EXTERN_C_ void mpi_reduce_local_(MPI_Fint *inbuf, MPI_Fint *inoutbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Reduce_local_fortran_wrapper(inbuf, inoutbuf, count, datatype, op, ierr);
}

_EXTERN_C_ void mpi_reduce_local__(MPI_Fint *inbuf, MPI_Fint *inoutbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *ierr) { 
    MPI_Reduce_local_fortran_wrapper(inbuf, inoutbuf, count, datatype, op, ierr);
}

/* ================= End Wrappers for MPI_Reduce_local ================= */


/* ================== C Wrappers for MPI_Reduce_scatter ================== */
_EXTERN_C_ int PMPI_Reduce_scatter(const void *sendbuf, void *recvbuf, const int recvcounts[], MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
_EXTERN_C_ int MPI_Reduce_scatter(const void *sendbuf, void *recvbuf, const int recvcounts[], MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Reduce_scatter(sendbuf, recvbuf, recvcounts, datatype, op, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Reduce_scatter =============== */
static void MPI_Reduce_scatter_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Reduce_scatter((const void*)sendbuf, (void*)recvbuf, (const int*)recvcounts, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Reduce_scatter((const void*)sendbuf, (void*)recvbuf, (const int*)recvcounts, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REDUCE_SCATTER(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(sendbuf, recvbuf, recvcounts, datatype, op, comm, ierr);
}

/* ================= End Wrappers for MPI_Reduce_scatter ================= */


/* ================== C Wrappers for MPI_Reduce_scatter_block ================== */
_EXTERN_C_ int PMPI_Reduce_scatter_block(const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
_EXTERN_C_ int MPI_Reduce_scatter_block(const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Reduce_scatter_block(sendbuf, recvbuf, recvcount, datatype, op, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Reduce_scatter_block =============== */
static void MPI_Reduce_scatter_block_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Reduce_scatter_block((const void*)sendbuf, (void*)recvbuf, *recvcount, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Reduce_scatter_block((const void*)sendbuf, (void*)recvbuf, *recvcount, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REDUCE_SCATTER_BLOCK(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter_block(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter_block_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter_block__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_block_fortran_wrapper(sendbuf, recvbuf, recvcount, datatype, op, comm, ierr);
}

/* ================= End Wrappers for MPI_Reduce_scatter_block ================= */


/* ================== C Wrappers for MPI_Register_datarep ================== */
_EXTERN_C_ int PMPI_Register_datarep(const char *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, void *extra_state);
_EXTERN_C_ int MPI_Register_datarep(const char *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, void *extra_state) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Register_datarep(datarep, read_conversion_fn, write_conversion_fn, dtype_file_extent_fn, extra_state);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Register_datarep =============== */
static void MPI_Register_datarep_fortran_wrapper(MPI_Fint *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Register_datarep((const char*)datarep, (MPI_Datarep_conversion_function*)read_conversion_fn, (MPI_Datarep_conversion_function*)write_conversion_fn, (MPI_Datarep_extent_function*)dtype_file_extent_fn, (void*)extra_state);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REGISTER_DATAREP(MPI_Fint *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Register_datarep_fortran_wrapper(datarep, read_conversion_fn, write_conversion_fn, dtype_file_extent_fn, extra_state, ierr);
}

_EXTERN_C_ void mpi_register_datarep(MPI_Fint *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Register_datarep_fortran_wrapper(datarep, read_conversion_fn, write_conversion_fn, dtype_file_extent_fn, extra_state, ierr);
}

_EXTERN_C_ void mpi_register_datarep_(MPI_Fint *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Register_datarep_fortran_wrapper(datarep, read_conversion_fn, write_conversion_fn, dtype_file_extent_fn, extra_state, ierr);
}

_EXTERN_C_ void mpi_register_datarep__(MPI_Fint *datarep, MPI_Datarep_conversion_function *read_conversion_fn, MPI_Datarep_conversion_function *write_conversion_fn, MPI_Datarep_extent_function *dtype_file_extent_fn, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Register_datarep_fortran_wrapper(datarep, read_conversion_fn, write_conversion_fn, dtype_file_extent_fn, extra_state, ierr);
}

/* ================= End Wrappers for MPI_Register_datarep ================= */


/* ================== C Wrappers for MPI_Request_free ================== */
_EXTERN_C_ int PMPI_Request_free(MPI_Request *request);
_EXTERN_C_ int MPI_Request_free(MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Request_free(request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Request_free =============== */
static void MPI_Request_free_fortran_wrapper(MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Request_free((MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Request_free(&temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REQUEST_FREE(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Request_free_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_request_free(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Request_free_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_request_free_(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Request_free_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_request_free__(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Request_free_fortran_wrapper(request, ierr);
}

/* ================= End Wrappers for MPI_Request_free ================= */


/* ================== C Wrappers for MPI_Request_get_status ================== */
_EXTERN_C_ int PMPI_Request_get_status(MPI_Request request, int *flag, MPI_Status *status);
_EXTERN_C_ int MPI_Request_get_status(MPI_Request request, int *flag, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Request_get_status(request, flag, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Request_get_status =============== */
static void MPI_Request_get_status_fortran_wrapper(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Request_get_status((MPI_Request)(*request), (int*)flag, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Request_get_status(MPI_Request_f2c(*request), (int*)flag, &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_REQUEST_GET_STATUS(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Request_get_status_fortran_wrapper(request, flag, status, ierr);
}

_EXTERN_C_ void mpi_request_get_status(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Request_get_status_fortran_wrapper(request, flag, status, ierr);
}

_EXTERN_C_ void mpi_request_get_status_(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Request_get_status_fortran_wrapper(request, flag, status, ierr);
}

_EXTERN_C_ void mpi_request_get_status__(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Request_get_status_fortran_wrapper(request, flag, status, ierr);
}

/* ================= End Wrappers for MPI_Request_get_status ================= */


/* ================== C Wrappers for MPI_Rget ================== */
_EXTERN_C_ int PMPI_Rget(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win, MPI_Request *request);
_EXTERN_C_ int MPI_Rget(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Rget(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Rget =============== */
static void MPI_Rget_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Rget((void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Win)(*win), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Rget((void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Win_f2c(*win), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RGET(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request, ierr);
}

_EXTERN_C_ void mpi_rget(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request, ierr);
}

_EXTERN_C_ void mpi_rget_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request, ierr);
}

_EXTERN_C_ void mpi_rget__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request, ierr);
}

/* ================= End Wrappers for MPI_Rget ================= */


/* ================== C Wrappers for MPI_Rget_accumulate ================== */
_EXTERN_C_ int PMPI_Rget_accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, void *result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win, MPI_Request *request);
_EXTERN_C_ int MPI_Rget_accumulate(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, void *result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPI_Win win, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Rget_accumulate(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Rget_accumulate =============== */
static void MPI_Rget_accumulate_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Rget_accumulate((const void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), (void*)result_addr, *result_count, (MPI_Datatype)(*result_datatype), *target_rank, *target_disp, *target_count, (MPI_Datatype)(*target_datatype), (MPI_Op)(*op), (MPI_Win)(*win), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Rget_accumulate((const void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), (void*)result_addr, *result_count, MPI_Type_f2c(*result_datatype), *target_rank, *target_disp, *target_count, MPI_Type_f2c(*target_datatype), MPI_Op_f2c(*op), MPI_Win_f2c(*win), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RGET_ACCUMULATE(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

_EXTERN_C_ void mpi_rget_accumulate(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

_EXTERN_C_ void mpi_rget_accumulate_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

_EXTERN_C_ void mpi_rget_accumulate__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *result_addr, MPI_Fint *result_count, MPI_Fint *result_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_count, MPI_Fint *target_datatype, MPI_Fint *op, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rget_accumulate_fortran_wrapper(origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request, ierr);
}

/* ================= End Wrappers for MPI_Rget_accumulate ================= */


/* ================== C Wrappers for MPI_Rput ================== */
_EXTERN_C_ int PMPI_Rput(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_cout, MPI_Datatype target_datatype, MPI_Win win, MPI_Request *request);
_EXTERN_C_ int MPI_Rput(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_cout, MPI_Datatype target_datatype, MPI_Win win, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Rput(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_cout, target_datatype, win, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Rput =============== */
static void MPI_Rput_fortran_wrapper(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_cout, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Rput((const void*)origin_addr, *origin_count, (MPI_Datatype)(*origin_datatype), *target_rank, *target_disp, *target_cout, (MPI_Datatype)(*target_datatype), (MPI_Win)(*win), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Rput((const void*)origin_addr, *origin_count, MPI_Type_f2c(*origin_datatype), *target_rank, *target_disp, *target_cout, MPI_Type_f2c(*target_datatype), MPI_Win_f2c(*win), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RPUT(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_cout, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rput_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_cout, target_datatype, win, request, ierr);
}

_EXTERN_C_ void mpi_rput(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_cout, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rput_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_cout, target_datatype, win, request, ierr);
}

_EXTERN_C_ void mpi_rput_(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_cout, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rput_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_cout, target_datatype, win, request, ierr);
}

_EXTERN_C_ void mpi_rput__(MPI_Fint *origin_addr, MPI_Fint *origin_count, MPI_Fint *origin_datatype, MPI_Fint *target_rank, MPI_Aint *target_disp, MPI_Fint *target_cout, MPI_Fint *target_datatype, MPI_Fint *win, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rput_fortran_wrapper(origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_cout, target_datatype, win, request, ierr);
}

/* ================= End Wrappers for MPI_Rput ================= */


/* ================== C Wrappers for MPI_Rsend ================== */
_EXTERN_C_ int PMPI_Rsend(const void *ibuf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Rsend(const void *ibuf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Rsend(ibuf, count, datatype, dest, tag, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Rsend =============== */
static void MPI_Rsend_fortran_wrapper(MPI_Fint *ibuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Rsend((const void*)ibuf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Rsend((const void*)ibuf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RSEND(MPI_Fint *ibuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Rsend_fortran_wrapper(ibuf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_rsend(MPI_Fint *ibuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Rsend_fortran_wrapper(ibuf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_rsend_(MPI_Fint *ibuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Rsend_fortran_wrapper(ibuf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_rsend__(MPI_Fint *ibuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Rsend_fortran_wrapper(ibuf, count, datatype, dest, tag, comm, ierr);
}

/* ================= End Wrappers for MPI_Rsend ================= */


/* ================== C Wrappers for MPI_Rsend_init ================== */
_EXTERN_C_ int PMPI_Rsend_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Rsend_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Rsend_init(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Rsend_init =============== */
static void MPI_Rsend_init_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Rsend_init((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Rsend_init((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_RSEND_INIT(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_rsend_init(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_rsend_init_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_rsend_init__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Rsend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Rsend_init ================= */


/* ================== C Wrappers for MPI_Scan ================== */
_EXTERN_C_ int PMPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
_EXTERN_C_ int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Scan(sendbuf, recvbuf, count, datatype, op, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Scan =============== */
static void MPI_Scan_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Scan((const void*)sendbuf, (void*)recvbuf, *count, (MPI_Datatype)(*datatype), (MPI_Op)(*op), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Scan((const void*)sendbuf, (void*)recvbuf, *count, MPI_Type_f2c(*datatype), MPI_Op_f2c(*op), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SCAN(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_scan(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_scan_(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

_EXTERN_C_ void mpi_scan__(MPI_Fint *sendbuf, MPI_Fint *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(sendbuf, recvbuf, count, datatype, op, comm, ierr);
}

/* ================= End Wrappers for MPI_Scan ================= */


/* ================== C Wrappers for MPI_Scatter ================== */
_EXTERN_C_ int PMPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
_EXTERN_C_ int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Scatter =============== */
static void MPI_Scatter_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Scatter((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Scatter((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SCATTER(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_scatter(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_scatter_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_scatter__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

/* ================= End Wrappers for MPI_Scatter ================= */


/* ================== C Wrappers for MPI_Scatterv ================== */
_EXTERN_C_ int PMPI_Scatterv(const void *sendbuf, const int sendcounts[], const int displs[], MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
_EXTERN_C_ int MPI_Scatterv(const void *sendbuf, const int sendcounts[], const int displs[], MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Scatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Scatterv =============== */
static void MPI_Scatterv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Scatterv((const void*)sendbuf, (const int*)sendcounts, (const int*)displs, (MPI_Datatype)(*sendtype), (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *root, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Scatterv((const void*)sendbuf, (const int*)sendcounts, (const int*)displs, MPI_Type_f2c(*sendtype), (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *root, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SCATTERV(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_scatterv(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_scatterv_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

_EXTERN_C_ void mpi_scatterv__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint displs[], MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, ierr);
}

/* ================= End Wrappers for MPI_Scatterv ================= */


/* ================== C Wrappers for MPI_Send ================== */
_EXTERN_C_ int PMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Send(buf, count, datatype, dest, tag, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Send =============== */
static void MPI_Send_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Send((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Send((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Send_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_send(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Send_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_send_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Send_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_send__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Send_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

/* ================= End Wrappers for MPI_Send ================= */


/* ================== C Wrappers for MPI_Send_init ================== */
_EXTERN_C_ int PMPI_Send_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Send_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Send_init(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Send_init =============== */
static void MPI_Send_init_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Send_init((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Send_init((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SEND_INIT(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Send_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_send_init(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Send_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_send_init_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Send_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_send_init__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Send_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Send_init ================= */


/* ================== C Wrappers for MPI_Sendrecv ================== */
_EXTERN_C_ int PMPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status);
_EXTERN_C_ int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Sendrecv(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Sendrecv =============== */
static void MPI_Sendrecv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Sendrecv((const void*)sendbuf, *sendcount, (MPI_Datatype)(*sendtype), *dest, *sendtag, (void*)recvbuf, *recvcount, (MPI_Datatype)(*recvtype), *source, *recvtag, (MPI_Comm)(*comm), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Sendrecv((const void*)sendbuf, *sendcount, MPI_Type_f2c(*sendtype), *dest, *sendtag, (void*)recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *source, *recvtag, MPI_Comm_f2c(*comm), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SENDRECV(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_fortran_wrapper(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status, ierr);
}

_EXTERN_C_ void mpi_sendrecv(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_fortran_wrapper(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status, ierr);
}

_EXTERN_C_ void mpi_sendrecv_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_fortran_wrapper(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status, ierr);
}

_EXTERN_C_ void mpi_sendrecv__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_fortran_wrapper(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status, ierr);
}

/* ================= End Wrappers for MPI_Sendrecv ================= */


/* ================== C Wrappers for MPI_Sendrecv_replace ================== */
_EXTERN_C_ int PMPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest, int sendtag, int source, int recvtag, MPI_Comm comm, MPI_Status *status);
_EXTERN_C_ int MPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest, int sendtag, int source, int recvtag, MPI_Comm comm, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Sendrecv_replace(buf, count, datatype, dest, sendtag, source, recvtag, comm, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Sendrecv_replace =============== */
static void MPI_Sendrecv_replace_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Sendrecv_replace((void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *sendtag, *source, *recvtag, (MPI_Comm)(*comm), (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Sendrecv_replace((void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *sendtag, *source, *recvtag, MPI_Comm_f2c(*comm), &temp_status);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SENDRECV_REPLACE(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_replace_fortran_wrapper(buf, count, datatype, dest, sendtag, source, recvtag, comm, status, ierr);
}

_EXTERN_C_ void mpi_sendrecv_replace(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_replace_fortran_wrapper(buf, count, datatype, dest, sendtag, source, recvtag, comm, status, ierr);
}

_EXTERN_C_ void mpi_sendrecv_replace_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_replace_fortran_wrapper(buf, count, datatype, dest, sendtag, source, recvtag, comm, status, ierr);
}

_EXTERN_C_ void mpi_sendrecv_replace__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *sendtag, MPI_Fint *source, MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Sendrecv_replace_fortran_wrapper(buf, count, datatype, dest, sendtag, source, recvtag, comm, status, ierr);
}

/* ================= End Wrappers for MPI_Sendrecv_replace ================= */


/* ================== C Wrappers for MPI_Ssend ================== */
_EXTERN_C_ int PMPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ssend(buf, count, datatype, dest, tag, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ssend =============== */
static void MPI_Ssend_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ssend((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Ssend((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SSEND(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Ssend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_ssend(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Ssend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_ssend_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Ssend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

_EXTERN_C_ void mpi_ssend__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Ssend_fortran_wrapper(buf, count, datatype, dest, tag, comm, ierr);
}

/* ================= End Wrappers for MPI_Ssend ================= */


/* ================== C Wrappers for MPI_Ssend_init ================== */
_EXTERN_C_ int PMPI_Ssend_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Ssend_init(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Ssend_init(buf, count, datatype, dest, tag, comm, request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Ssend_init =============== */
static void MPI_Ssend_init_fortran_wrapper(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Ssend_init((const void*)buf, *count, (MPI_Datatype)(*datatype), *dest, *tag, (MPI_Comm)(*comm), (MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Ssend_init((const void*)buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_SSEND_INIT(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ssend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_ssend_init(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ssend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_ssend_init_(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ssend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

_EXTERN_C_ void mpi_ssend_init__(MPI_Fint *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Ssend_init_fortran_wrapper(buf, count, datatype, dest, tag, comm, request, ierr);
}

/* ================= End Wrappers for MPI_Ssend_init ================= */


/* ================== C Wrappers for MPI_Start ================== */
_EXTERN_C_ int PMPI_Start(MPI_Request *request);
_EXTERN_C_ int MPI_Start(MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Start(request);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Start =============== */
static void MPI_Start_fortran_wrapper(MPI_Fint *request, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Start((MPI_Request*)request);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    temp_request = MPI_Request_f2c(*request);
    _wrap_py_return_val = MPI_Start(&temp_request);
    *request = MPI_Request_c2f(temp_request);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_START(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Start_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_start(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Start_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_start_(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Start_fortran_wrapper(request, ierr);
}

_EXTERN_C_ void mpi_start__(MPI_Fint *request, MPI_Fint *ierr) { 
    MPI_Start_fortran_wrapper(request, ierr);
}

/* ================= End Wrappers for MPI_Start ================= */


/* ================== C Wrappers for MPI_Startall ================== */
_EXTERN_C_ int PMPI_Startall(int count, MPI_Request array_of_requests[]);
_EXTERN_C_ int MPI_Startall(int count, MPI_Request array_of_requests[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Startall(count, array_of_requests);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Startall =============== */
static void MPI_Startall_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Startall(*count, (MPI_Request*)array_of_requests);
#else /* MPI-2 safe call */
    MPI_Request* temp_array_of_requests;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *count);
    for (i=0; i < *count; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    _wrap_py_return_val = MPI_Startall(*count, temp_array_of_requests);
    for (i=0; i < *count; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_STARTALL(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *ierr) { 
    MPI_Startall_fortran_wrapper(count, array_of_requests, ierr);
}

_EXTERN_C_ void mpi_startall(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *ierr) { 
    MPI_Startall_fortran_wrapper(count, array_of_requests, ierr);
}

_EXTERN_C_ void mpi_startall_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *ierr) { 
    MPI_Startall_fortran_wrapper(count, array_of_requests, ierr);
}

_EXTERN_C_ void mpi_startall__(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *ierr) { 
    MPI_Startall_fortran_wrapper(count, array_of_requests, ierr);
}

/* ================= End Wrappers for MPI_Startall ================= */


/* ================== C Wrappers for MPI_Status_set_cancelled ================== */
_EXTERN_C_ int PMPI_Status_set_cancelled(MPI_Status *status, int flag);
_EXTERN_C_ int MPI_Status_set_cancelled(MPI_Status *status, int flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Status_set_cancelled(status, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Status_set_cancelled =============== */
static void MPI_Status_set_cancelled_fortran_wrapper(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Status_set_cancelled((MPI_Status*)status, *flag);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Status_set_cancelled(&temp_status, *flag);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_STATUS_SET_CANCELLED(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Status_set_cancelled_fortran_wrapper(status, flag, ierr);
}

_EXTERN_C_ void mpi_status_set_cancelled(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Status_set_cancelled_fortran_wrapper(status, flag, ierr);
}

_EXTERN_C_ void mpi_status_set_cancelled_(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Status_set_cancelled_fortran_wrapper(status, flag, ierr);
}

_EXTERN_C_ void mpi_status_set_cancelled__(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Status_set_cancelled_fortran_wrapper(status, flag, ierr);
}

/* ================= End Wrappers for MPI_Status_set_cancelled ================= */


/* ================== C Wrappers for MPI_Status_set_elements ================== */
_EXTERN_C_ int PMPI_Status_set_elements(MPI_Status *status, MPI_Datatype datatype, int count);
_EXTERN_C_ int MPI_Status_set_elements(MPI_Status *status, MPI_Datatype datatype, int count) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Status_set_elements(status, datatype, count);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Status_set_elements =============== */
static void MPI_Status_set_elements_fortran_wrapper(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Status_set_elements((MPI_Status*)status, (MPI_Datatype)(*datatype), *count);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Status_set_elements(&temp_status, MPI_Type_f2c(*datatype), *count);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_STATUS_SET_ELEMENTS(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_status_set_elements(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_status_set_elements_(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_status_set_elements__(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_fortran_wrapper(status, datatype, count, ierr);
}

/* ================= End Wrappers for MPI_Status_set_elements ================= */


/* ================== C Wrappers for MPI_Status_set_elements_x ================== */
_EXTERN_C_ int PMPI_Status_set_elements_x(MPI_Status *status, MPI_Datatype datatype, MPI_Count count);
_EXTERN_C_ int MPI_Status_set_elements_x(MPI_Status *status, MPI_Datatype datatype, MPI_Count count) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Status_set_elements_x(status, datatype, count);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Status_set_elements_x =============== */
static void MPI_Status_set_elements_x_fortran_wrapper(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Status_set_elements_x((MPI_Status*)status, (MPI_Datatype)(*datatype), *count);
#else /* MPI-2 safe call */
    MPI_Status temp_status;
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Status_set_elements_x(&temp_status, MPI_Type_f2c(*datatype), *count);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_STATUS_SET_ELEMENTS_X(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_status_set_elements_x(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_status_set_elements_x_(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

_EXTERN_C_ void mpi_status_set_elements_x__(MPI_Fint *status, MPI_Fint *datatype, MPI_Fint *count, MPI_Fint *ierr) { 
    MPI_Status_set_elements_x_fortran_wrapper(status, datatype, count, ierr);
}

/* ================= End Wrappers for MPI_Status_set_elements_x ================= */


/* ================== C Wrappers for MPI_Test ================== */
_EXTERN_C_ int PMPI_Test(MPI_Request *request, int *flag, MPI_Status *status);
_EXTERN_C_ int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Test(request, flag, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Test =============== */
static void MPI_Test_fortran_wrapper(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Test((MPI_Request*)request, (int*)flag, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    MPI_Status temp_status;
    temp_request = MPI_Request_f2c(*request);
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Test(&temp_request, (int*)flag, &temp_status);
    *request = MPI_Request_c2f(temp_request);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TEST(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Test_fortran_wrapper(request, flag, status, ierr);
}

_EXTERN_C_ void mpi_test(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Test_fortran_wrapper(request, flag, status, ierr);
}

_EXTERN_C_ void mpi_test_(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Test_fortran_wrapper(request, flag, status, ierr);
}

_EXTERN_C_ void mpi_test__(MPI_Fint *request, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Test_fortran_wrapper(request, flag, status, ierr);
}

/* ================= End Wrappers for MPI_Test ================= */


/* ================== C Wrappers for MPI_Test_cancelled ================== */
_EXTERN_C_ int PMPI_Test_cancelled(const MPI_Status *status, int *flag);
_EXTERN_C_ int MPI_Test_cancelled(const MPI_Status *status, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Test_cancelled(status, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Test_cancelled =============== */
static void MPI_Test_cancelled_fortran_wrapper(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Test_cancelled((const MPI_Status*)status, (int*)flag);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TEST_CANCELLED(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Test_cancelled_fortran_wrapper(status, flag, ierr);
}

_EXTERN_C_ void mpi_test_cancelled(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Test_cancelled_fortran_wrapper(status, flag, ierr);
}

_EXTERN_C_ void mpi_test_cancelled_(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Test_cancelled_fortran_wrapper(status, flag, ierr);
}

_EXTERN_C_ void mpi_test_cancelled__(MPI_Fint *status, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Test_cancelled_fortran_wrapper(status, flag, ierr);
}

/* ================= End Wrappers for MPI_Test_cancelled ================= */


/* ================== C Wrappers for MPI_Testall ================== */
_EXTERN_C_ int PMPI_Testall(int count, MPI_Request array_of_requests[], int *flag, MPI_Status array_of_statuses[]);
_EXTERN_C_ int MPI_Testall(int count, MPI_Request array_of_requests[], int *flag, MPI_Status array_of_statuses[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Testall(count, array_of_requests, flag, array_of_statuses);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Testall =============== */
static void MPI_Testall_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *flag, MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Testall(*count, (MPI_Request*)array_of_requests, (int*)flag, (MPI_Status*)array_of_statuses);
#else /* MPI-2 safe call */
    MPI_Status* temp_array_of_statuses;
    MPI_Request* temp_array_of_requests;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *count);
    for (i=0; i < *count; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    temp_array_of_statuses = (MPI_Status*)malloc(sizeof(MPI_Status) * *count);
    for (i=0; i < *count; i++)
        MPI_Status_f2c(&array_of_statuses[i], &temp_array_of_statuses[i]);
    _wrap_py_return_val = MPI_Testall(*count, temp_array_of_requests, (int*)flag, temp_array_of_statuses);
    for (i=0; i < *count; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
    for (i=0; i < *count; i++)
        MPI_Status_c2f(&temp_array_of_statuses[i], &array_of_statuses[i]);
    free(temp_array_of_statuses);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TESTALL(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *flag, MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testall_fortran_wrapper(count, array_of_requests, flag, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_testall(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *flag, MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testall_fortran_wrapper(count, array_of_requests, flag, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_testall_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *flag, MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testall_fortran_wrapper(count, array_of_requests, flag, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_testall__(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *flag, MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testall_fortran_wrapper(count, array_of_requests, flag, array_of_statuses, ierr);
}

/* ================= End Wrappers for MPI_Testall ================= */


/* ================== C Wrappers for MPI_Testany ================== */
_EXTERN_C_ int PMPI_Testany(int count, MPI_Request array_of_requests[], int *index, int *flag, MPI_Status *status);
_EXTERN_C_ int MPI_Testany(int count, MPI_Request array_of_requests[], int *index, int *flag, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Testany(count, array_of_requests, index, flag, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Testany =============== */
static void MPI_Testany_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Testany(*count, (MPI_Request*)array_of_requests, (int*)index, (int*)flag, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Request* temp_array_of_requests;
    MPI_Status temp_status;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *count);
    for (i=0; i < *count; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Testany(*count, temp_array_of_requests, (int*)index, (int*)flag, &temp_status);
    for (i=0; i < *count; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TESTANY(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Testany_fortran_wrapper(count, array_of_requests, index, flag, status, ierr);
}

_EXTERN_C_ void mpi_testany(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Testany_fortran_wrapper(count, array_of_requests, index, flag, status, ierr);
}

_EXTERN_C_ void mpi_testany_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Testany_fortran_wrapper(count, array_of_requests, index, flag, status, ierr);
}

_EXTERN_C_ void mpi_testany__(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *flag, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Testany_fortran_wrapper(count, array_of_requests, index, flag, status, ierr);
}

/* ================= End Wrappers for MPI_Testany ================= */


/* ================== C Wrappers for MPI_Testsome ================== */
_EXTERN_C_ int PMPI_Testsome(int incount, MPI_Request array_of_requests[], int *outcount, int array_of_indices[], MPI_Status array_of_statuses[]);
_EXTERN_C_ int MPI_Testsome(int incount, MPI_Request array_of_requests[], int *outcount, int array_of_indices[], MPI_Status array_of_statuses[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Testsome(incount, array_of_requests, outcount, array_of_indices, array_of_statuses);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Testsome =============== */
static void MPI_Testsome_fortran_wrapper(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Testsome(*incount, (MPI_Request*)array_of_requests, (int*)outcount, (int*)array_of_indices, (MPI_Status*)array_of_statuses);
#else /* MPI-2 safe call */
    MPI_Status* temp_array_of_statuses;
    MPI_Request* temp_array_of_requests;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *incount);
    for (i=0; i < *incount; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    temp_array_of_statuses = (MPI_Status*)malloc(sizeof(MPI_Status) * *incount);
    for (i=0; i < *incount; i++)
        MPI_Status_f2c(&array_of_statuses[i], &temp_array_of_statuses[i]);
    _wrap_py_return_val = MPI_Testsome(*incount, temp_array_of_requests, (int*)outcount, (int*)array_of_indices, temp_array_of_statuses);
    for (i=0; i < *incount; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
    for (i=0; i < *incount; i++)
        MPI_Status_c2f(&temp_array_of_statuses[i], &array_of_statuses[i]);
    free(temp_array_of_statuses);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TESTSOME(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_testsome(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_testsome_(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_testsome__(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Testsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

/* ================= End Wrappers for MPI_Testsome ================= */


/* ================== C Wrappers for MPI_Topo_test ================== */
_EXTERN_C_ int PMPI_Topo_test(MPI_Comm comm, int *status);
_EXTERN_C_ int MPI_Topo_test(MPI_Comm comm, int *status) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Topo_test(comm, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Topo_test =============== */
static void MPI_Topo_test_fortran_wrapper(MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Topo_test((MPI_Comm)(*comm), (int*)status);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Topo_test(MPI_Comm_f2c(*comm), (int*)status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TOPO_TEST(MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Topo_test_fortran_wrapper(comm, status, ierr);
}

_EXTERN_C_ void mpi_topo_test(MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Topo_test_fortran_wrapper(comm, status, ierr);
}

_EXTERN_C_ void mpi_topo_test_(MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Topo_test_fortran_wrapper(comm, status, ierr);
}

_EXTERN_C_ void mpi_topo_test__(MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Topo_test_fortran_wrapper(comm, status, ierr);
}

/* ================= End Wrappers for MPI_Topo_test ================= */


/* ================== C Wrappers for MPI_Type_commit ================== */
_EXTERN_C_ int PMPI_Type_commit(MPI_Datatype *type);
_EXTERN_C_ int MPI_Type_commit(MPI_Datatype *type) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_commit(type);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_commit =============== */
static void MPI_Type_commit_fortran_wrapper(MPI_Fint *type, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_commit((MPI_Datatype*)type);
#else /* MPI-2 safe call */
    MPI_Datatype temp_type;
    temp_type = MPI_Type_f2c(*type);
    _wrap_py_return_val = MPI_Type_commit(&temp_type);
    *type = MPI_Type_c2f(temp_type);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_COMMIT(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_commit_fortran_wrapper(type, ierr);
}

_EXTERN_C_ void mpi_type_commit(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_commit_fortran_wrapper(type, ierr);
}

_EXTERN_C_ void mpi_type_commit_(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_commit_fortran_wrapper(type, ierr);
}

_EXTERN_C_ void mpi_type_commit__(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_commit_fortran_wrapper(type, ierr);
}

/* ================= End Wrappers for MPI_Type_commit ================= */


/* ================== C Wrappers for MPI_Type_contiguous ================== */
_EXTERN_C_ int PMPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_contiguous(count, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_contiguous =============== */
static void MPI_Type_contiguous_fortran_wrapper(MPI_Fint *count, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_contiguous(*count, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_contiguous(*count, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CONTIGUOUS(MPI_Fint *count, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_contiguous_fortran_wrapper(count, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_contiguous(MPI_Fint *count, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_contiguous_fortran_wrapper(count, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_contiguous_(MPI_Fint *count, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_contiguous_fortran_wrapper(count, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_contiguous__(MPI_Fint *count, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_contiguous_fortran_wrapper(count, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_contiguous ================= */


/* ================== C Wrappers for MPI_Type_create_darray ================== */
_EXTERN_C_ int PMPI_Type_create_darray(int size, int rank, int ndims, const int gsize_array[], const int distrib_array[], const int darg_array[], const int psize_array[], int order, MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_darray(int size, int rank, int ndims, const int gsize_array[], const int distrib_array[], const int darg_array[], const int psize_array[], int order, MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_darray(size, rank, ndims, gsize_array, distrib_array, darg_array, psize_array, order, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_darray =============== */
static void MPI_Type_create_darray_fortran_wrapper(MPI_Fint *size, MPI_Fint *rank, MPI_Fint *ndims, MPI_Fint gsize_array[], MPI_Fint distrib_array[], MPI_Fint darg_array[], MPI_Fint psize_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_darray(*size, *rank, *ndims, (const int*)gsize_array, (const int*)distrib_array, (const int*)darg_array, (const int*)psize_array, *order, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_darray(*size, *rank, *ndims, (const int*)gsize_array, (const int*)distrib_array, (const int*)darg_array, (const int*)psize_array, *order, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_DARRAY(MPI_Fint *size, MPI_Fint *rank, MPI_Fint *ndims, MPI_Fint gsize_array[], MPI_Fint distrib_array[], MPI_Fint darg_array[], MPI_Fint psize_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_darray_fortran_wrapper(size, rank, ndims, gsize_array, distrib_array, darg_array, psize_array, order, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_darray(MPI_Fint *size, MPI_Fint *rank, MPI_Fint *ndims, MPI_Fint gsize_array[], MPI_Fint distrib_array[], MPI_Fint darg_array[], MPI_Fint psize_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_darray_fortran_wrapper(size, rank, ndims, gsize_array, distrib_array, darg_array, psize_array, order, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_darray_(MPI_Fint *size, MPI_Fint *rank, MPI_Fint *ndims, MPI_Fint gsize_array[], MPI_Fint distrib_array[], MPI_Fint darg_array[], MPI_Fint psize_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_darray_fortran_wrapper(size, rank, ndims, gsize_array, distrib_array, darg_array, psize_array, order, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_darray__(MPI_Fint *size, MPI_Fint *rank, MPI_Fint *ndims, MPI_Fint gsize_array[], MPI_Fint distrib_array[], MPI_Fint darg_array[], MPI_Fint psize_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_darray_fortran_wrapper(size, rank, ndims, gsize_array, distrib_array, darg_array, psize_array, order, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_darray ================= */


/* ================== C Wrappers for MPI_Type_create_f90_complex ================== */
_EXTERN_C_ int PMPI_Type_create_f90_complex(int p, int r, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_f90_complex(int p, int r, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_f90_complex(p, r, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_f90_complex =============== */
static void MPI_Type_create_f90_complex_fortran_wrapper(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_f90_complex(*p, *r, (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_f90_complex(*p, *r, &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_F90_COMPLEX(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_complex_fortran_wrapper(p, r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_complex(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_complex_fortran_wrapper(p, r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_complex_(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_complex_fortran_wrapper(p, r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_complex__(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_complex_fortran_wrapper(p, r, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_f90_complex ================= */


/* ================== C Wrappers for MPI_Type_create_f90_integer ================== */
_EXTERN_C_ int PMPI_Type_create_f90_integer(int r, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_f90_integer(int r, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_f90_integer(r, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_f90_integer =============== */
static void MPI_Type_create_f90_integer_fortran_wrapper(MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_f90_integer(*r, (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_f90_integer(*r, &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_F90_INTEGER(MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_integer_fortran_wrapper(r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_integer(MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_integer_fortran_wrapper(r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_integer_(MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_integer_fortran_wrapper(r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_integer__(MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_integer_fortran_wrapper(r, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_f90_integer ================= */


/* ================== C Wrappers for MPI_Type_create_f90_real ================== */
_EXTERN_C_ int PMPI_Type_create_f90_real(int p, int r, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_f90_real(int p, int r, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_f90_real(p, r, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_f90_real =============== */
static void MPI_Type_create_f90_real_fortran_wrapper(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_f90_real(*p, *r, (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_f90_real(*p, *r, &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_F90_REAL(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_real_fortran_wrapper(p, r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_real(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_real_fortran_wrapper(p, r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_real_(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_real_fortran_wrapper(p, r, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_f90_real__(MPI_Fint *p, MPI_Fint *r, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_f90_real_fortran_wrapper(p, r, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_f90_real ================= */


/* ================== C Wrappers for MPI_Type_create_hindexed ================== */
_EXTERN_C_ int PMPI_Type_create_hindexed(int count, const int array_of_blocklengths[], const MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_hindexed(int count, const int array_of_blocklengths[], const MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_hindexed(count, array_of_blocklengths, array_of_displacements, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_hindexed =============== */
static void MPI_Type_create_hindexed_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_hindexed(*count, (const int*)array_of_blocklengths, (const MPI_Aint*)array_of_displacements, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_hindexed(*count, (const int*)array_of_blocklengths, (const MPI_Aint*)array_of_displacements, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_HINDEXED(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hindexed(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hindexed_(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hindexed__(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_hindexed ================= */


/* ================== C Wrappers for MPI_Type_create_hindexed_block ================== */
_EXTERN_C_ int PMPI_Type_create_hindexed_block(int count, int blocklength, const MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_hindexed_block(int count, int blocklength, const MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_hindexed_block(count, blocklength, array_of_displacements, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_hindexed_block =============== */
static void MPI_Type_create_hindexed_block_fortran_wrapper(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_hindexed_block(*count, *blocklength, (const MPI_Aint*)array_of_displacements, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_hindexed_block(*count, *blocklength, (const MPI_Aint*)array_of_displacements, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_HINDEXED_BLOCK(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hindexed_block(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hindexed_block_(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hindexed_block__(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hindexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_hindexed_block ================= */


/* ================== C Wrappers for MPI_Type_create_hvector ================== */
_EXTERN_C_ int PMPI_Type_create_hvector(int count, int blocklength, MPI_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_hvector(int count, int blocklength, MPI_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_hvector(count, blocklength, stride, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_hvector =============== */
static void MPI_Type_create_hvector_fortran_wrapper(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_hvector(*count, *blocklength, *stride, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_hvector(*count, *blocklength, *stride, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_HVECTOR(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hvector(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hvector_(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_hvector__(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_hvector ================= */


/* ================== C Wrappers for MPI_Type_create_indexed_block ================== */
_EXTERN_C_ int PMPI_Type_create_indexed_block(int count, int blocklength, const int array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_indexed_block(int count, int blocklength, const int array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_indexed_block(count, blocklength, array_of_displacements, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_indexed_block =============== */
static void MPI_Type_create_indexed_block_fortran_wrapper(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_indexed_block(*count, *blocklength, (const int*)array_of_displacements, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_indexed_block(*count, *blocklength, (const int*)array_of_displacements, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_INDEXED_BLOCK(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_indexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_indexed_block(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_indexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_indexed_block_(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_indexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_indexed_block__(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_indexed_block_fortran_wrapper(count, blocklength, array_of_displacements, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_indexed_block ================= */


/* ================== C Wrappers for MPI_Type_create_keyval ================== */
_EXTERN_C_ int PMPI_Type_create_keyval(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, int *type_keyval, void *extra_state);
_EXTERN_C_ int MPI_Type_create_keyval(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, int *type_keyval, void *extra_state) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_keyval(type_copy_attr_fn, type_delete_attr_fn, type_keyval, extra_state);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_keyval =============== */
static void MPI_Type_create_keyval_fortran_wrapper(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, MPI_Fint *type_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Type_create_keyval((MPI_Type_copy_attr_function*)type_copy_attr_fn, (MPI_Type_delete_attr_function*)type_delete_attr_fn, (int*)type_keyval, (void*)extra_state);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_KEYVAL(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, MPI_Fint *type_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Type_create_keyval_fortran_wrapper(type_copy_attr_fn, type_delete_attr_fn, type_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_type_create_keyval(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, MPI_Fint *type_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Type_create_keyval_fortran_wrapper(type_copy_attr_fn, type_delete_attr_fn, type_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_type_create_keyval_(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, MPI_Fint *type_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Type_create_keyval_fortran_wrapper(type_copy_attr_fn, type_delete_attr_fn, type_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_type_create_keyval__(MPI_Type_copy_attr_function *type_copy_attr_fn, MPI_Type_delete_attr_function *type_delete_attr_fn, MPI_Fint *type_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Type_create_keyval_fortran_wrapper(type_copy_attr_fn, type_delete_attr_fn, type_keyval, extra_state, ierr);
}

/* ================= End Wrappers for MPI_Type_create_keyval ================= */


/* ================== C Wrappers for MPI_Type_create_resized ================== */
_EXTERN_C_ int PMPI_Type_create_resized(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_resized(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_resized(oldtype, lb, extent, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_resized =============== */
static void MPI_Type_create_resized_fortran_wrapper(MPI_Fint *oldtype, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_resized((MPI_Datatype)(*oldtype), *lb, *extent, (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_resized(MPI_Type_f2c(*oldtype), *lb, *extent, &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_RESIZED(MPI_Fint *oldtype, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_resized_fortran_wrapper(oldtype, lb, extent, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_resized(MPI_Fint *oldtype, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_resized_fortran_wrapper(oldtype, lb, extent, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_resized_(MPI_Fint *oldtype, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_resized_fortran_wrapper(oldtype, lb, extent, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_resized__(MPI_Fint *oldtype, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_resized_fortran_wrapper(oldtype, lb, extent, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_resized ================= */


/* ================== C Wrappers for MPI_Type_create_struct ================== */
_EXTERN_C_ int PMPI_Type_create_struct(int count, const int array_of_block_lengths[], const MPI_Aint array_of_displacements[], const MPI_Datatype array_of_types[], MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_struct(int count, const int array_of_block_lengths[], const MPI_Aint array_of_displacements[], const MPI_Datatype array_of_types[], MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_struct(count, array_of_block_lengths, array_of_displacements, array_of_types, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_struct =============== */
static void MPI_Type_create_struct_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_block_lengths[], MPI_Fint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_struct(*count, (const int*)array_of_block_lengths, (const MPI_Aint*)array_of_displacements, (const MPI_Datatype*)array_of_types, (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_struct(*count, (const int*)array_of_block_lengths, (const MPI_Aint*)array_of_displacements, (const MPI_Datatype*)array_of_types, &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_STRUCT(MPI_Fint *count, MPI_Fint array_of_block_lengths[], MPI_Fint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_struct_fortran_wrapper(count, array_of_block_lengths, array_of_displacements, array_of_types, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_struct(MPI_Fint *count, MPI_Fint array_of_block_lengths[], MPI_Fint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_struct_fortran_wrapper(count, array_of_block_lengths, array_of_displacements, array_of_types, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_struct_(MPI_Fint *count, MPI_Fint array_of_block_lengths[], MPI_Fint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_struct_fortran_wrapper(count, array_of_block_lengths, array_of_displacements, array_of_types, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_struct__(MPI_Fint *count, MPI_Fint array_of_block_lengths[], MPI_Fint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_struct_fortran_wrapper(count, array_of_block_lengths, array_of_displacements, array_of_types, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_struct ================= */


/* ================== C Wrappers for MPI_Type_create_subarray ================== */
_EXTERN_C_ int PMPI_Type_create_subarray(int ndims, const int size_array[], const int subsize_array[], const int start_array[], int order, MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_create_subarray(int ndims, const int size_array[], const int subsize_array[], const int start_array[], int order, MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_create_subarray(ndims, size_array, subsize_array, start_array, order, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_create_subarray =============== */
static void MPI_Type_create_subarray_fortran_wrapper(MPI_Fint *ndims, MPI_Fint size_array[], MPI_Fint subsize_array[], MPI_Fint start_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_create_subarray(*ndims, (const int*)size_array, (const int*)subsize_array, (const int*)start_array, *order, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_create_subarray(*ndims, (const int*)size_array, (const int*)subsize_array, (const int*)start_array, *order, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_CREATE_SUBARRAY(MPI_Fint *ndims, MPI_Fint size_array[], MPI_Fint subsize_array[], MPI_Fint start_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_subarray_fortran_wrapper(ndims, size_array, subsize_array, start_array, order, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_subarray(MPI_Fint *ndims, MPI_Fint size_array[], MPI_Fint subsize_array[], MPI_Fint start_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_subarray_fortran_wrapper(ndims, size_array, subsize_array, start_array, order, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_subarray_(MPI_Fint *ndims, MPI_Fint size_array[], MPI_Fint subsize_array[], MPI_Fint start_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_subarray_fortran_wrapper(ndims, size_array, subsize_array, start_array, order, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_create_subarray__(MPI_Fint *ndims, MPI_Fint size_array[], MPI_Fint subsize_array[], MPI_Fint start_array[], MPI_Fint *order, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_create_subarray_fortran_wrapper(ndims, size_array, subsize_array, start_array, order, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_create_subarray ================= */


/* ================== C Wrappers for MPI_Type_delete_attr ================== */
_EXTERN_C_ int PMPI_Type_delete_attr(MPI_Datatype type, int type_keyval);
_EXTERN_C_ int MPI_Type_delete_attr(MPI_Datatype type, int type_keyval) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_delete_attr(type, type_keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_delete_attr =============== */
static void MPI_Type_delete_attr_fortran_wrapper(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_delete_attr((MPI_Datatype)(*type), *type_keyval);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_delete_attr(MPI_Type_f2c(*type), *type_keyval);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_DELETE_ATTR(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_delete_attr_fortran_wrapper(type, type_keyval, ierr);
}

_EXTERN_C_ void mpi_type_delete_attr(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_delete_attr_fortran_wrapper(type, type_keyval, ierr);
}

_EXTERN_C_ void mpi_type_delete_attr_(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_delete_attr_fortran_wrapper(type, type_keyval, ierr);
}

_EXTERN_C_ void mpi_type_delete_attr__(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_delete_attr_fortran_wrapper(type, type_keyval, ierr);
}

/* ================= End Wrappers for MPI_Type_delete_attr ================= */


/* ================== C Wrappers for MPI_Type_dup ================== */
_EXTERN_C_ int PMPI_Type_dup(MPI_Datatype type, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_dup(MPI_Datatype type, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_dup(type, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_dup =============== */
static void MPI_Type_dup_fortran_wrapper(MPI_Fint *type, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_dup((MPI_Datatype)(*type), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_dup(MPI_Type_f2c(*type), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_DUP(MPI_Fint *type, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_dup_fortran_wrapper(type, newtype, ierr);
}

_EXTERN_C_ void mpi_type_dup(MPI_Fint *type, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_dup_fortran_wrapper(type, newtype, ierr);
}

_EXTERN_C_ void mpi_type_dup_(MPI_Fint *type, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_dup_fortran_wrapper(type, newtype, ierr);
}

_EXTERN_C_ void mpi_type_dup__(MPI_Fint *type, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_dup_fortran_wrapper(type, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_dup ================= */


/* ================== C Wrappers for MPI_Type_extent ================== */
_EXTERN_C_ int PMPI_Type_extent(MPI_Datatype type, MPI_Aint *extent);
_EXTERN_C_ int MPI_Type_extent(MPI_Datatype type, MPI_Aint *extent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_extent(type, extent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_extent =============== */
static void MPI_Type_extent_fortran_wrapper(MPI_Fint *type, MPI_Aint *extent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_extent((MPI_Datatype)(*type), (MPI_Aint*)extent);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_extent(MPI_Type_f2c(*type), (MPI_Aint*)extent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_EXTENT(MPI_Fint *type, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_extent_fortran_wrapper(type, extent, ierr);
}

_EXTERN_C_ void mpi_type_extent(MPI_Fint *type, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_extent_fortran_wrapper(type, extent, ierr);
}

_EXTERN_C_ void mpi_type_extent_(MPI_Fint *type, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_extent_fortran_wrapper(type, extent, ierr);
}

_EXTERN_C_ void mpi_type_extent__(MPI_Fint *type, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_extent_fortran_wrapper(type, extent, ierr);
}

/* ================= End Wrappers for MPI_Type_extent ================= */


/* ================== C Wrappers for MPI_Type_free ================== */
_EXTERN_C_ int PMPI_Type_free(MPI_Datatype *type);
_EXTERN_C_ int MPI_Type_free(MPI_Datatype *type) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_free(type);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_free =============== */
static void MPI_Type_free_fortran_wrapper(MPI_Fint *type, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_free((MPI_Datatype*)type);
#else /* MPI-2 safe call */
    MPI_Datatype temp_type;
    temp_type = MPI_Type_f2c(*type);
    _wrap_py_return_val = MPI_Type_free(&temp_type);
    *type = MPI_Type_c2f(temp_type);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_FREE(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_free_fortran_wrapper(type, ierr);
}

_EXTERN_C_ void mpi_type_free(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_free_fortran_wrapper(type, ierr);
}

_EXTERN_C_ void mpi_type_free_(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_free_fortran_wrapper(type, ierr);
}

_EXTERN_C_ void mpi_type_free__(MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_free_fortran_wrapper(type, ierr);
}

/* ================= End Wrappers for MPI_Type_free ================= */


/* ================== C Wrappers for MPI_Type_free_keyval ================== */
_EXTERN_C_ int PMPI_Type_free_keyval(int *type_keyval);
_EXTERN_C_ int MPI_Type_free_keyval(int *type_keyval) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_free_keyval(type_keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_free_keyval =============== */
static void MPI_Type_free_keyval_fortran_wrapper(MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Type_free_keyval((int*)type_keyval);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_FREE_KEYVAL(MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_free_keyval_fortran_wrapper(type_keyval, ierr);
}

_EXTERN_C_ void mpi_type_free_keyval(MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_free_keyval_fortran_wrapper(type_keyval, ierr);
}

_EXTERN_C_ void mpi_type_free_keyval_(MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_free_keyval_fortran_wrapper(type_keyval, ierr);
}

_EXTERN_C_ void mpi_type_free_keyval__(MPI_Fint *type_keyval, MPI_Fint *ierr) { 
    MPI_Type_free_keyval_fortran_wrapper(type_keyval, ierr);
}

/* ================= End Wrappers for MPI_Type_free_keyval ================= */


/* ================== C Wrappers for MPI_Type_get_attr ================== */
_EXTERN_C_ int PMPI_Type_get_attr(MPI_Datatype type, int type_keyval, void *attribute_val, int *flag);
_EXTERN_C_ int MPI_Type_get_attr(MPI_Datatype type, int type_keyval, void *attribute_val, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_attr(type, type_keyval, attribute_val, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_attr =============== */
static void MPI_Type_get_attr_fortran_wrapper(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_attr((MPI_Datatype)(*type), *type_keyval, (void*)attribute_val, (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_attr(MPI_Type_f2c(*type), *type_keyval, (void*)attribute_val, (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_ATTR(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Type_get_attr_fortran_wrapper(type, type_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_type_get_attr(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Type_get_attr_fortran_wrapper(type, type_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_type_get_attr_(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Type_get_attr_fortran_wrapper(type, type_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_type_get_attr__(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Type_get_attr_fortran_wrapper(type, type_keyval, attribute_val, flag, ierr);
}

/* ================= End Wrappers for MPI_Type_get_attr ================= */


/* ================== C Wrappers for MPI_Type_get_contents ================== */
_EXTERN_C_ int PMPI_Type_get_contents(MPI_Datatype mtype, int max_integers, int max_addresses, int max_datatypes, int array_of_integers[], MPI_Aint array_of_addresses[], MPI_Datatype array_of_datatypes[]);
_EXTERN_C_ int MPI_Type_get_contents(MPI_Datatype mtype, int max_integers, int max_addresses, int max_datatypes, int array_of_integers[], MPI_Aint array_of_addresses[], MPI_Datatype array_of_datatypes[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_contents(mtype, max_integers, max_addresses, max_datatypes, array_of_integers, array_of_addresses, array_of_datatypes);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_contents =============== */
static void MPI_Type_get_contents_fortran_wrapper(MPI_Fint *mtype, MPI_Fint *max_integers, MPI_Fint *max_addresses, MPI_Fint *max_datatypes, MPI_Fint array_of_integers[], MPI_Aint array_of_addresses[], MPI_Fint array_of_datatypes[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_contents((MPI_Datatype)(*mtype), *max_integers, *max_addresses, *max_datatypes, (int*)array_of_integers, (MPI_Aint*)array_of_addresses, (MPI_Datatype*)array_of_datatypes);
#else /* MPI-2 safe call */
    MPI_Datatype* temp_array_of_datatypes;
    int i;
    temp_array_of_datatypes = (MPI_Datatype*)malloc(sizeof(MPI_Datatype) * *max_integers);
    for (i=0; i < *max_integers; i++)
        temp_array_of_datatypes[i] = MPI_Type_f2c(array_of_datatypes[i]);
    _wrap_py_return_val = MPI_Type_get_contents(MPI_Type_f2c(*mtype), *max_integers, *max_addresses, *max_datatypes, (int*)array_of_integers, (MPI_Aint*)array_of_addresses, temp_array_of_datatypes);
    for (i=0; i < *max_integers; i++)
        array_of_datatypes[i] = MPI_Type_c2f(temp_array_of_datatypes[i]);
    free(temp_array_of_datatypes);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_CONTENTS(MPI_Fint *mtype, MPI_Fint *max_integers, MPI_Fint *max_addresses, MPI_Fint *max_datatypes, MPI_Fint array_of_integers[], MPI_Aint array_of_addresses[], MPI_Fint array_of_datatypes[], MPI_Fint *ierr) { 
    MPI_Type_get_contents_fortran_wrapper(mtype, max_integers, max_addresses, max_datatypes, array_of_integers, array_of_addresses, array_of_datatypes, ierr);
}

_EXTERN_C_ void mpi_type_get_contents(MPI_Fint *mtype, MPI_Fint *max_integers, MPI_Fint *max_addresses, MPI_Fint *max_datatypes, MPI_Fint array_of_integers[], MPI_Aint array_of_addresses[], MPI_Fint array_of_datatypes[], MPI_Fint *ierr) { 
    MPI_Type_get_contents_fortran_wrapper(mtype, max_integers, max_addresses, max_datatypes, array_of_integers, array_of_addresses, array_of_datatypes, ierr);
}

_EXTERN_C_ void mpi_type_get_contents_(MPI_Fint *mtype, MPI_Fint *max_integers, MPI_Fint *max_addresses, MPI_Fint *max_datatypes, MPI_Fint array_of_integers[], MPI_Aint array_of_addresses[], MPI_Fint array_of_datatypes[], MPI_Fint *ierr) { 
    MPI_Type_get_contents_fortran_wrapper(mtype, max_integers, max_addresses, max_datatypes, array_of_integers, array_of_addresses, array_of_datatypes, ierr);
}

_EXTERN_C_ void mpi_type_get_contents__(MPI_Fint *mtype, MPI_Fint *max_integers, MPI_Fint *max_addresses, MPI_Fint *max_datatypes, MPI_Fint array_of_integers[], MPI_Aint array_of_addresses[], MPI_Fint array_of_datatypes[], MPI_Fint *ierr) { 
    MPI_Type_get_contents_fortran_wrapper(mtype, max_integers, max_addresses, max_datatypes, array_of_integers, array_of_addresses, array_of_datatypes, ierr);
}

/* ================= End Wrappers for MPI_Type_get_contents ================= */


/* ================== C Wrappers for MPI_Type_get_envelope ================== */
_EXTERN_C_ int PMPI_Type_get_envelope(MPI_Datatype type, int *num_integers, int *num_addresses, int *num_datatypes, int *combiner);
_EXTERN_C_ int MPI_Type_get_envelope(MPI_Datatype type, int *num_integers, int *num_addresses, int *num_datatypes, int *combiner) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_envelope(type, num_integers, num_addresses, num_datatypes, combiner);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_envelope =============== */
static void MPI_Type_get_envelope_fortran_wrapper(MPI_Fint *type, MPI_Fint *num_integers, MPI_Fint *num_addresses, MPI_Fint *num_datatypes, MPI_Fint *combiner, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_envelope((MPI_Datatype)(*type), (int*)num_integers, (int*)num_addresses, (int*)num_datatypes, (int*)combiner);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_envelope(MPI_Type_f2c(*type), (int*)num_integers, (int*)num_addresses, (int*)num_datatypes, (int*)combiner);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_ENVELOPE(MPI_Fint *type, MPI_Fint *num_integers, MPI_Fint *num_addresses, MPI_Fint *num_datatypes, MPI_Fint *combiner, MPI_Fint *ierr) { 
    MPI_Type_get_envelope_fortran_wrapper(type, num_integers, num_addresses, num_datatypes, combiner, ierr);
}

_EXTERN_C_ void mpi_type_get_envelope(MPI_Fint *type, MPI_Fint *num_integers, MPI_Fint *num_addresses, MPI_Fint *num_datatypes, MPI_Fint *combiner, MPI_Fint *ierr) { 
    MPI_Type_get_envelope_fortran_wrapper(type, num_integers, num_addresses, num_datatypes, combiner, ierr);
}

_EXTERN_C_ void mpi_type_get_envelope_(MPI_Fint *type, MPI_Fint *num_integers, MPI_Fint *num_addresses, MPI_Fint *num_datatypes, MPI_Fint *combiner, MPI_Fint *ierr) { 
    MPI_Type_get_envelope_fortran_wrapper(type, num_integers, num_addresses, num_datatypes, combiner, ierr);
}

_EXTERN_C_ void mpi_type_get_envelope__(MPI_Fint *type, MPI_Fint *num_integers, MPI_Fint *num_addresses, MPI_Fint *num_datatypes, MPI_Fint *combiner, MPI_Fint *ierr) { 
    MPI_Type_get_envelope_fortran_wrapper(type, num_integers, num_addresses, num_datatypes, combiner, ierr);
}

/* ================= End Wrappers for MPI_Type_get_envelope ================= */


/* ================== C Wrappers for MPI_Type_get_extent ================== */
_EXTERN_C_ int PMPI_Type_get_extent(MPI_Datatype type, MPI_Aint *lb, MPI_Aint *extent);
_EXTERN_C_ int MPI_Type_get_extent(MPI_Datatype type, MPI_Aint *lb, MPI_Aint *extent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_extent(type, lb, extent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_extent =============== */
static void MPI_Type_get_extent_fortran_wrapper(MPI_Fint *type, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_extent((MPI_Datatype)(*type), (MPI_Aint*)lb, (MPI_Aint*)extent);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_extent(MPI_Type_f2c(*type), (MPI_Aint*)lb, (MPI_Aint*)extent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_EXTENT(MPI_Fint *type, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_fortran_wrapper(type, lb, extent, ierr);
}

_EXTERN_C_ void mpi_type_get_extent(MPI_Fint *type, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_fortran_wrapper(type, lb, extent, ierr);
}

_EXTERN_C_ void mpi_type_get_extent_(MPI_Fint *type, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_fortran_wrapper(type, lb, extent, ierr);
}

_EXTERN_C_ void mpi_type_get_extent__(MPI_Fint *type, MPI_Aint *lb, MPI_Aint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_fortran_wrapper(type, lb, extent, ierr);
}

/* ================= End Wrappers for MPI_Type_get_extent ================= */


/* ================== C Wrappers for MPI_Type_get_extent_x ================== */
_EXTERN_C_ int PMPI_Type_get_extent_x(MPI_Datatype type, MPI_Count *lb, MPI_Count *extent);
_EXTERN_C_ int MPI_Type_get_extent_x(MPI_Datatype type, MPI_Count *lb, MPI_Count *extent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_extent_x(type, lb, extent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_extent_x =============== */
static void MPI_Type_get_extent_x_fortran_wrapper(MPI_Fint *type, MPI_Fint *lb, MPI_Fint *extent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_extent_x((MPI_Datatype)(*type), (MPI_Count*)lb, (MPI_Count*)extent);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_extent_x(MPI_Type_f2c(*type), (MPI_Count*)lb, (MPI_Count*)extent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_EXTENT_X(MPI_Fint *type, MPI_Fint *lb, MPI_Fint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_x_fortran_wrapper(type, lb, extent, ierr);
}

_EXTERN_C_ void mpi_type_get_extent_x(MPI_Fint *type, MPI_Fint *lb, MPI_Fint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_x_fortran_wrapper(type, lb, extent, ierr);
}

_EXTERN_C_ void mpi_type_get_extent_x_(MPI_Fint *type, MPI_Fint *lb, MPI_Fint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_x_fortran_wrapper(type, lb, extent, ierr);
}

_EXTERN_C_ void mpi_type_get_extent_x__(MPI_Fint *type, MPI_Fint *lb, MPI_Fint *extent, MPI_Fint *ierr) { 
    MPI_Type_get_extent_x_fortran_wrapper(type, lb, extent, ierr);
}

/* ================= End Wrappers for MPI_Type_get_extent_x ================= */


/* ================== C Wrappers for MPI_Type_get_name ================== */
_EXTERN_C_ int PMPI_Type_get_name(MPI_Datatype type, char *type_name, int *resultlen);
_EXTERN_C_ int MPI_Type_get_name(MPI_Datatype type, char *type_name, int *resultlen) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_name(type, type_name, resultlen);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_name =============== */
static void MPI_Type_get_name_fortran_wrapper(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_name((MPI_Datatype)(*type), (char*)type_name, (int*)resultlen);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_name(MPI_Type_f2c(*type), (char*)type_name, (int*)resultlen);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_NAME(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Type_get_name_fortran_wrapper(type, type_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_type_get_name(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Type_get_name_fortran_wrapper(type, type_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_type_get_name_(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Type_get_name_fortran_wrapper(type, type_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_type_get_name__(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Type_get_name_fortran_wrapper(type, type_name, resultlen, ierr);
}

/* ================= End Wrappers for MPI_Type_get_name ================= */


/* ================== C Wrappers for MPI_Type_get_true_extent ================== */
_EXTERN_C_ int PMPI_Type_get_true_extent(MPI_Datatype datatype, MPI_Aint *true_lb, MPI_Aint *true_extent);
_EXTERN_C_ int MPI_Type_get_true_extent(MPI_Datatype datatype, MPI_Aint *true_lb, MPI_Aint *true_extent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_true_extent(datatype, true_lb, true_extent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_true_extent =============== */
static void MPI_Type_get_true_extent_fortran_wrapper(MPI_Fint *datatype, MPI_Aint *true_lb, MPI_Aint *true_extent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_true_extent((MPI_Datatype)(*datatype), (MPI_Aint*)true_lb, (MPI_Aint*)true_extent);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_true_extent(MPI_Type_f2c(*datatype), (MPI_Aint*)true_lb, (MPI_Aint*)true_extent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_TRUE_EXTENT(MPI_Fint *datatype, MPI_Aint *true_lb, MPI_Aint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

_EXTERN_C_ void mpi_type_get_true_extent(MPI_Fint *datatype, MPI_Aint *true_lb, MPI_Aint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

_EXTERN_C_ void mpi_type_get_true_extent_(MPI_Fint *datatype, MPI_Aint *true_lb, MPI_Aint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

_EXTERN_C_ void mpi_type_get_true_extent__(MPI_Fint *datatype, MPI_Aint *true_lb, MPI_Aint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

/* ================= End Wrappers for MPI_Type_get_true_extent ================= */


/* ================== C Wrappers for MPI_Type_get_true_extent_x ================== */
_EXTERN_C_ int PMPI_Type_get_true_extent_x(MPI_Datatype datatype, MPI_Count *true_lb, MPI_Count *true_extent);
_EXTERN_C_ int MPI_Type_get_true_extent_x(MPI_Datatype datatype, MPI_Count *true_lb, MPI_Count *true_extent) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_get_true_extent_x(datatype, true_lb, true_extent);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_get_true_extent_x =============== */
static void MPI_Type_get_true_extent_x_fortran_wrapper(MPI_Fint *datatype, MPI_Fint *true_lb, MPI_Fint *true_extent, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_get_true_extent_x((MPI_Datatype)(*datatype), (MPI_Count*)true_lb, (MPI_Count*)true_extent);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_get_true_extent_x(MPI_Type_f2c(*datatype), (MPI_Count*)true_lb, (MPI_Count*)true_extent);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_GET_TRUE_EXTENT_X(MPI_Fint *datatype, MPI_Fint *true_lb, MPI_Fint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_x_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

_EXTERN_C_ void mpi_type_get_true_extent_x(MPI_Fint *datatype, MPI_Fint *true_lb, MPI_Fint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_x_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

_EXTERN_C_ void mpi_type_get_true_extent_x_(MPI_Fint *datatype, MPI_Fint *true_lb, MPI_Fint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_x_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

_EXTERN_C_ void mpi_type_get_true_extent_x__(MPI_Fint *datatype, MPI_Fint *true_lb, MPI_Fint *true_extent, MPI_Fint *ierr) { 
    MPI_Type_get_true_extent_x_fortran_wrapper(datatype, true_lb, true_extent, ierr);
}

/* ================= End Wrappers for MPI_Type_get_true_extent_x ================= */


/* ================== C Wrappers for MPI_Type_hindexed ================== */
_EXTERN_C_ int PMPI_Type_hindexed(int count, int array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_hindexed(int count, int array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_hindexed(count, array_of_blocklengths, array_of_displacements, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_hindexed =============== */
static void MPI_Type_hindexed_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_hindexed(*count, (int*)array_of_blocklengths, (MPI_Aint*)array_of_displacements, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_hindexed(*count, (int*)array_of_blocklengths, (MPI_Aint*)array_of_displacements, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_HINDEXED(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_hindexed(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_hindexed_(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_hindexed__(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hindexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_hindexed ================= */


/* ================== C Wrappers for MPI_Type_hvector ================== */
_EXTERN_C_ int PMPI_Type_hvector(int count, int blocklength, MPI_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_hvector(int count, int blocklength, MPI_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_hvector(count, blocklength, stride, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_hvector =============== */
static void MPI_Type_hvector_fortran_wrapper(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_hvector(*count, *blocklength, *stride, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_hvector(*count, *blocklength, *stride, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_HVECTOR(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_hvector(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_hvector_(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_hvector__(MPI_Fint *count, MPI_Fint *blocklength, MPI_Aint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_hvector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_hvector ================= */


/* ================== C Wrappers for MPI_Type_indexed ================== */
_EXTERN_C_ int PMPI_Type_indexed(int count, const int array_of_blocklengths[], const int array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_indexed(int count, const int array_of_blocklengths[], const int array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_indexed(count, array_of_blocklengths, array_of_displacements, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_indexed =============== */
static void MPI_Type_indexed_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_indexed(*count, (const int*)array_of_blocklengths, (const int*)array_of_displacements, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_indexed(*count, (const int*)array_of_blocklengths, (const int*)array_of_displacements, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_INDEXED(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_indexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_indexed(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_indexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_indexed_(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_indexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_indexed__(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Fint array_of_displacements[], MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_indexed_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_indexed ================= */


/* ================== C Wrappers for MPI_Type_lb ================== */
_EXTERN_C_ int PMPI_Type_lb(MPI_Datatype type, MPI_Aint *lb);
_EXTERN_C_ int MPI_Type_lb(MPI_Datatype type, MPI_Aint *lb) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_lb(type, lb);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_lb =============== */
static void MPI_Type_lb_fortran_wrapper(MPI_Fint *type, MPI_Aint *lb, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_lb((MPI_Datatype)(*type), (MPI_Aint*)lb);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_lb(MPI_Type_f2c(*type), (MPI_Aint*)lb);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_LB(MPI_Fint *type, MPI_Aint *lb, MPI_Fint *ierr) { 
    MPI_Type_lb_fortran_wrapper(type, lb, ierr);
}

_EXTERN_C_ void mpi_type_lb(MPI_Fint *type, MPI_Aint *lb, MPI_Fint *ierr) { 
    MPI_Type_lb_fortran_wrapper(type, lb, ierr);
}

_EXTERN_C_ void mpi_type_lb_(MPI_Fint *type, MPI_Aint *lb, MPI_Fint *ierr) { 
    MPI_Type_lb_fortran_wrapper(type, lb, ierr);
}

_EXTERN_C_ void mpi_type_lb__(MPI_Fint *type, MPI_Aint *lb, MPI_Fint *ierr) { 
    MPI_Type_lb_fortran_wrapper(type, lb, ierr);
}

/* ================= End Wrappers for MPI_Type_lb ================= */


/* ================== C Wrappers for MPI_Type_match_size ================== */
_EXTERN_C_ int PMPI_Type_match_size(int typeclass, int size, MPI_Datatype *type);
_EXTERN_C_ int MPI_Type_match_size(int typeclass, int size, MPI_Datatype *type) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_match_size(typeclass, size, type);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_match_size =============== */
static void MPI_Type_match_size_fortran_wrapper(MPI_Fint *typeclass, MPI_Fint *size, MPI_Fint *type, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_match_size(*typeclass, *size, (MPI_Datatype*)type);
#else /* MPI-2 safe call */
    MPI_Datatype temp_type;
    temp_type = MPI_Type_f2c(*type);
    _wrap_py_return_val = MPI_Type_match_size(*typeclass, *size, &temp_type);
    *type = MPI_Type_c2f(temp_type);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_MATCH_SIZE(MPI_Fint *typeclass, MPI_Fint *size, MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_match_size_fortran_wrapper(typeclass, size, type, ierr);
}

_EXTERN_C_ void mpi_type_match_size(MPI_Fint *typeclass, MPI_Fint *size, MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_match_size_fortran_wrapper(typeclass, size, type, ierr);
}

_EXTERN_C_ void mpi_type_match_size_(MPI_Fint *typeclass, MPI_Fint *size, MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_match_size_fortran_wrapper(typeclass, size, type, ierr);
}

_EXTERN_C_ void mpi_type_match_size__(MPI_Fint *typeclass, MPI_Fint *size, MPI_Fint *type, MPI_Fint *ierr) { 
    MPI_Type_match_size_fortran_wrapper(typeclass, size, type, ierr);
}

/* ================= End Wrappers for MPI_Type_match_size ================= */


/* ================== C Wrappers for MPI_Type_set_attr ================== */
_EXTERN_C_ int PMPI_Type_set_attr(MPI_Datatype type, int type_keyval, void *attr_val);
_EXTERN_C_ int MPI_Type_set_attr(MPI_Datatype type, int type_keyval, void *attr_val) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_set_attr(type, type_keyval, attr_val);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_set_attr =============== */
static void MPI_Type_set_attr_fortran_wrapper(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attr_val, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_set_attr((MPI_Datatype)(*type), *type_keyval, (void*)attr_val);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_set_attr(MPI_Type_f2c(*type), *type_keyval, (void*)attr_val);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_SET_ATTR(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attr_val, MPI_Fint *ierr) { 
    MPI_Type_set_attr_fortran_wrapper(type, type_keyval, attr_val, ierr);
}

_EXTERN_C_ void mpi_type_set_attr(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attr_val, MPI_Fint *ierr) { 
    MPI_Type_set_attr_fortran_wrapper(type, type_keyval, attr_val, ierr);
}

_EXTERN_C_ void mpi_type_set_attr_(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attr_val, MPI_Fint *ierr) { 
    MPI_Type_set_attr_fortran_wrapper(type, type_keyval, attr_val, ierr);
}

_EXTERN_C_ void mpi_type_set_attr__(MPI_Fint *type, MPI_Fint *type_keyval, MPI_Fint *attr_val, MPI_Fint *ierr) { 
    MPI_Type_set_attr_fortran_wrapper(type, type_keyval, attr_val, ierr);
}

/* ================= End Wrappers for MPI_Type_set_attr ================= */


/* ================== C Wrappers for MPI_Type_set_name ================== */
_EXTERN_C_ int PMPI_Type_set_name(MPI_Datatype type, const char *type_name);
_EXTERN_C_ int MPI_Type_set_name(MPI_Datatype type, const char *type_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_set_name(type, type_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_set_name =============== */
static void MPI_Type_set_name_fortran_wrapper(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_set_name((MPI_Datatype)(*type), (const char*)type_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_set_name(MPI_Type_f2c(*type), (const char*)type_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_SET_NAME(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *ierr) { 
    MPI_Type_set_name_fortran_wrapper(type, type_name, ierr);
}

_EXTERN_C_ void mpi_type_set_name(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *ierr) { 
    MPI_Type_set_name_fortran_wrapper(type, type_name, ierr);
}

_EXTERN_C_ void mpi_type_set_name_(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *ierr) { 
    MPI_Type_set_name_fortran_wrapper(type, type_name, ierr);
}

_EXTERN_C_ void mpi_type_set_name__(MPI_Fint *type, MPI_Fint *type_name, MPI_Fint *ierr) { 
    MPI_Type_set_name_fortran_wrapper(type, type_name, ierr);
}

/* ================= End Wrappers for MPI_Type_set_name ================= */


/* ================== C Wrappers for MPI_Type_size ================== */
_EXTERN_C_ int PMPI_Type_size(MPI_Datatype type, int *size);
_EXTERN_C_ int MPI_Type_size(MPI_Datatype type, int *size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_size(type, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_size =============== */
static void MPI_Type_size_fortran_wrapper(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_size((MPI_Datatype)(*type), (int*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_size(MPI_Type_f2c(*type), (int*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_SIZE(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_fortran_wrapper(type, size, ierr);
}

_EXTERN_C_ void mpi_type_size(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_fortran_wrapper(type, size, ierr);
}

_EXTERN_C_ void mpi_type_size_(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_fortran_wrapper(type, size, ierr);
}

_EXTERN_C_ void mpi_type_size__(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_fortran_wrapper(type, size, ierr);
}

/* ================= End Wrappers for MPI_Type_size ================= */


/* ================== C Wrappers for MPI_Type_size_x ================== */
_EXTERN_C_ int PMPI_Type_size_x(MPI_Datatype type, MPI_Count *size);
_EXTERN_C_ int MPI_Type_size_x(MPI_Datatype type, MPI_Count *size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_size_x(type, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_size_x =============== */
static void MPI_Type_size_x_fortran_wrapper(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_size_x((MPI_Datatype)(*type), (MPI_Count*)size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_size_x(MPI_Type_f2c(*type), (MPI_Count*)size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_SIZE_X(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_x_fortran_wrapper(type, size, ierr);
}

_EXTERN_C_ void mpi_type_size_x(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_x_fortran_wrapper(type, size, ierr);
}

_EXTERN_C_ void mpi_type_size_x_(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_x_fortran_wrapper(type, size, ierr);
}

_EXTERN_C_ void mpi_type_size_x__(MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) { 
    MPI_Type_size_x_fortran_wrapper(type, size, ierr);
}

/* ================= End Wrappers for MPI_Type_size_x ================= */


/* ================== C Wrappers for MPI_Type_struct ================== */
_EXTERN_C_ int PMPI_Type_struct(int count, int array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Datatype array_of_types[], MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_struct(int count, int array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Datatype array_of_types[], MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_struct =============== */
static void MPI_Type_struct_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_struct(*count, (int*)array_of_blocklengths, (MPI_Aint*)array_of_displacements, (MPI_Datatype*)array_of_types, (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    MPI_Datatype* temp_array_of_types;
    int i;
    temp_array_of_types = (MPI_Datatype*)malloc(sizeof(MPI_Datatype) * *count);
    for (i=0; i < *count; i++)
        temp_array_of_types[i] = MPI_Type_f2c(array_of_types[i]);
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_struct(*count, (int*)array_of_blocklengths, (MPI_Aint*)array_of_displacements, temp_array_of_types, &temp_newtype);
    for (i=0; i < *count; i++)
        array_of_types[i] = MPI_Type_c2f(temp_array_of_types[i]);
    free(temp_array_of_types);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_STRUCT(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_struct_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype, ierr);
}

_EXTERN_C_ void mpi_type_struct(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_struct_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype, ierr);
}

_EXTERN_C_ void mpi_type_struct_(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_struct_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype, ierr);
}

_EXTERN_C_ void mpi_type_struct__(MPI_Fint *count, MPI_Fint array_of_blocklengths[], MPI_Aint array_of_displacements[], MPI_Fint array_of_types[], MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_struct_fortran_wrapper(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_struct ================= */


/* ================== C Wrappers for MPI_Type_ub ================== */
_EXTERN_C_ int PMPI_Type_ub(MPI_Datatype mtype, MPI_Aint *ub);
_EXTERN_C_ int MPI_Type_ub(MPI_Datatype mtype, MPI_Aint *ub) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_ub(mtype, ub);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_ub =============== */
static void MPI_Type_ub_fortran_wrapper(MPI_Fint *mtype, MPI_Aint *ub, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_ub((MPI_Datatype)(*mtype), (MPI_Aint*)ub);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Type_ub(MPI_Type_f2c(*mtype), (MPI_Aint*)ub);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_UB(MPI_Fint *mtype, MPI_Aint *ub, MPI_Fint *ierr) { 
    MPI_Type_ub_fortran_wrapper(mtype, ub, ierr);
}

_EXTERN_C_ void mpi_type_ub(MPI_Fint *mtype, MPI_Aint *ub, MPI_Fint *ierr) { 
    MPI_Type_ub_fortran_wrapper(mtype, ub, ierr);
}

_EXTERN_C_ void mpi_type_ub_(MPI_Fint *mtype, MPI_Aint *ub, MPI_Fint *ierr) { 
    MPI_Type_ub_fortran_wrapper(mtype, ub, ierr);
}

_EXTERN_C_ void mpi_type_ub__(MPI_Fint *mtype, MPI_Aint *ub, MPI_Fint *ierr) { 
    MPI_Type_ub_fortran_wrapper(mtype, ub, ierr);
}

/* ================= End Wrappers for MPI_Type_ub ================= */


/* ================== C Wrappers for MPI_Type_vector ================== */
_EXTERN_C_ int PMPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
_EXTERN_C_ int MPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Type_vector(count, blocklength, stride, oldtype, newtype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Type_vector =============== */
static void MPI_Type_vector_fortran_wrapper(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Type_vector(*count, *blocklength, *stride, (MPI_Datatype)(*oldtype), (MPI_Datatype*)newtype);
#else /* MPI-2 safe call */
    MPI_Datatype temp_newtype;
    temp_newtype = MPI_Type_f2c(*newtype);
    _wrap_py_return_val = MPI_Type_vector(*count, *blocklength, *stride, MPI_Type_f2c(*oldtype), &temp_newtype);
    *newtype = MPI_Type_c2f(temp_newtype);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_TYPE_VECTOR(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_vector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_vector(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_vector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_vector_(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_vector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

_EXTERN_C_ void mpi_type_vector__(MPI_Fint *count, MPI_Fint *blocklength, MPI_Fint *stride, MPI_Fint *oldtype, MPI_Fint *newtype, MPI_Fint *ierr) { 
    MPI_Type_vector_fortran_wrapper(count, blocklength, stride, oldtype, newtype, ierr);
}

/* ================= End Wrappers for MPI_Type_vector ================= */


/* ================== C Wrappers for MPI_Unpack ================== */
_EXTERN_C_ int PMPI_Unpack(const void *inbuf, int insize, int *position, void *outbuf, int outcount, MPI_Datatype datatype, MPI_Comm comm);
_EXTERN_C_ int MPI_Unpack(const void *inbuf, int insize, int *position, void *outbuf, int outcount, MPI_Datatype datatype, MPI_Comm comm) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Unpack(inbuf, insize, position, outbuf, outcount, datatype, comm);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Unpack =============== */
static void MPI_Unpack_fortran_wrapper(MPI_Fint *inbuf, MPI_Fint *insize, MPI_Fint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Unpack((const void*)inbuf, *insize, (int*)position, (void*)outbuf, *outcount, (MPI_Datatype)(*datatype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Unpack((const void*)inbuf, *insize, (int*)position, (void*)outbuf, *outcount, MPI_Type_f2c(*datatype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_UNPACK(MPI_Fint *inbuf, MPI_Fint *insize, MPI_Fint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Unpack_fortran_wrapper(inbuf, insize, position, outbuf, outcount, datatype, comm, ierr);
}

_EXTERN_C_ void mpi_unpack(MPI_Fint *inbuf, MPI_Fint *insize, MPI_Fint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Unpack_fortran_wrapper(inbuf, insize, position, outbuf, outcount, datatype, comm, ierr);
}

_EXTERN_C_ void mpi_unpack_(MPI_Fint *inbuf, MPI_Fint *insize, MPI_Fint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Unpack_fortran_wrapper(inbuf, insize, position, outbuf, outcount, datatype, comm, ierr);
}

_EXTERN_C_ void mpi_unpack__(MPI_Fint *inbuf, MPI_Fint *insize, MPI_Fint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Unpack_fortran_wrapper(inbuf, insize, position, outbuf, outcount, datatype, comm, ierr);
}

/* ================= End Wrappers for MPI_Unpack ================= */


/* ================== C Wrappers for MPI_Unpack_external ================== */
_EXTERN_C_ int PMPI_Unpack_external(const char datarep[], const void *inbuf, MPI_Aint insize, MPI_Aint *position, void *outbuf, int outcount, MPI_Datatype datatype);
_EXTERN_C_ int MPI_Unpack_external(const char datarep[], const void *inbuf, MPI_Aint insize, MPI_Aint *position, void *outbuf, int outcount, MPI_Datatype datatype) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Unpack_external(datarep, inbuf, insize, position, outbuf, outcount, datatype);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Unpack_external =============== */
static void MPI_Unpack_external_fortran_wrapper(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Aint *insize, MPI_Aint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Unpack_external((const char*)datarep, (const void*)inbuf, *insize, (MPI_Aint*)position, (void*)outbuf, *outcount, (MPI_Datatype)(*datatype));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Unpack_external((const char*)datarep, (const void*)inbuf, *insize, (MPI_Aint*)position, (void*)outbuf, *outcount, MPI_Type_f2c(*datatype));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_UNPACK_EXTERNAL(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Aint *insize, MPI_Aint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_Unpack_external_fortran_wrapper(datarep, inbuf, insize, position, outbuf, outcount, datatype, ierr);
}

_EXTERN_C_ void mpi_unpack_external(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Aint *insize, MPI_Aint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_Unpack_external_fortran_wrapper(datarep, inbuf, insize, position, outbuf, outcount, datatype, ierr);
}

_EXTERN_C_ void mpi_unpack_external_(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Aint *insize, MPI_Aint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_Unpack_external_fortran_wrapper(datarep, inbuf, insize, position, outbuf, outcount, datatype, ierr);
}

_EXTERN_C_ void mpi_unpack_external__(MPI_Fint datarep[], MPI_Fint *inbuf, MPI_Aint *insize, MPI_Aint *position, MPI_Fint *outbuf, MPI_Fint *outcount, MPI_Fint *datatype, MPI_Fint *ierr) { 
    MPI_Unpack_external_fortran_wrapper(datarep, inbuf, insize, position, outbuf, outcount, datatype, ierr);
}

/* ================= End Wrappers for MPI_Unpack_external ================= */


/* ================== C Wrappers for MPI_Unpublish_name ================== */
_EXTERN_C_ int PMPI_Unpublish_name(const char *service_name, MPI_Info info, const char *port_name);
_EXTERN_C_ int MPI_Unpublish_name(const char *service_name, MPI_Info info, const char *port_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Unpublish_name(service_name, info, port_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Unpublish_name =============== */
static void MPI_Unpublish_name_fortran_wrapper(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Unpublish_name((const char*)service_name, (MPI_Info)(*info), (const char*)port_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Unpublish_name((const char*)service_name, MPI_Info_f2c(*info), (const char*)port_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_UNPUBLISH_NAME(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Unpublish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_unpublish_name(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Unpublish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_unpublish_name_(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Unpublish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

_EXTERN_C_ void mpi_unpublish_name__(MPI_Fint *service_name, MPI_Fint *info, MPI_Fint *port_name, MPI_Fint *ierr) { 
    MPI_Unpublish_name_fortran_wrapper(service_name, info, port_name, ierr);
}

/* ================= End Wrappers for MPI_Unpublish_name ================= */


/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int PMPI_Wait(MPI_Request *request, MPI_Status *status);
_EXTERN_C_ int MPI_Wait(MPI_Request *request, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Wait(request, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Wait =============== */
static void MPI_Wait_fortran_wrapper(MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Wait((MPI_Request*)request, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Request temp_request;
    MPI_Status temp_status;
    temp_request = MPI_Request_f2c(*request);
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Wait(&temp_request, &temp_status);
    *request = MPI_Request_c2f(temp_request);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WAIT(MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(request, status, ierr);
}

_EXTERN_C_ void mpi_wait(MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(request, status, ierr);
}

_EXTERN_C_ void mpi_wait_(MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(request, status, ierr);
}

_EXTERN_C_ void mpi_wait__(MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(request, status, ierr);
}

/* ================= End Wrappers for MPI_Wait ================= */


/* ================== C Wrappers for MPI_Waitall ================== */
_EXTERN_C_ int PMPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status *array_of_statuses);
_EXTERN_C_ int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status *array_of_statuses) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Waitall(count, array_of_requests, array_of_statuses);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Waitall =============== */
static void MPI_Waitall_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *array_of_statuses, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Waitall(*count, (MPI_Request*)array_of_requests, (MPI_Status*)array_of_statuses);
#else /* MPI-2 safe call */
    MPI_Status* temp_array_of_statuses;
    MPI_Request* temp_array_of_requests;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *count);
    for (i=0; i < *count; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    temp_array_of_statuses = (MPI_Status*)malloc(sizeof(MPI_Status) * *count);
    for (i=0; i < *count; i++)
        MPI_Status_f2c(&array_of_statuses[i], &temp_array_of_statuses[i]);
    _wrap_py_return_val = MPI_Waitall(*count, temp_array_of_requests, temp_array_of_statuses);
    for (i=0; i < *count; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
    for (i=0; i < *count; i++)
        MPI_Status_c2f(&temp_array_of_statuses[i], &array_of_statuses[i]);
    free(temp_array_of_statuses);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WAITALL(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *array_of_statuses, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(count, array_of_requests, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_waitall(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *array_of_statuses, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(count, array_of_requests, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_waitall_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *array_of_statuses, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(count, array_of_requests, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_waitall__(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *array_of_statuses, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(count, array_of_requests, array_of_statuses, ierr);
}

/* ================= End Wrappers for MPI_Waitall ================= */


/* ================== C Wrappers for MPI_Waitany ================== */
_EXTERN_C_ int PMPI_Waitany(int count, MPI_Request array_of_requests[], int *index, MPI_Status *status);
_EXTERN_C_ int MPI_Waitany(int count, MPI_Request array_of_requests[], int *index, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Waitany(count, array_of_requests, index, status);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Waitany =============== */
static void MPI_Waitany_fortran_wrapper(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *status, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Waitany(*count, (MPI_Request*)array_of_requests, (int*)index, (MPI_Status*)status);
#else /* MPI-2 safe call */
    MPI_Request* temp_array_of_requests;
    MPI_Status temp_status;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *count);
    for (i=0; i < *count; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    MPI_Status_f2c(status, &temp_status);
    _wrap_py_return_val = MPI_Waitany(*count, temp_array_of_requests, (int*)index, &temp_status);
    for (i=0; i < *count; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
    MPI_Status_c2f(&temp_status, status);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WAITANY(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(count, array_of_requests, index, status, ierr);
}

_EXTERN_C_ void mpi_waitany(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(count, array_of_requests, index, status, ierr);
}

_EXTERN_C_ void mpi_waitany_(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(count, array_of_requests, index, status, ierr);
}

_EXTERN_C_ void mpi_waitany__(MPI_Fint *count, MPI_Fint array_of_requests[], MPI_Fint *index, MPI_Fint *status, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(count, array_of_requests, index, status, ierr);
}

/* ================= End Wrappers for MPI_Waitany ================= */


/* ================== C Wrappers for MPI_Waitsome ================== */
_EXTERN_C_ int PMPI_Waitsome(int incount, MPI_Request array_of_requests[], int *outcount, int array_of_indices[], MPI_Status array_of_statuses[]);
_EXTERN_C_ int MPI_Waitsome(int incount, MPI_Request array_of_requests[], int *outcount, int array_of_indices[], MPI_Status array_of_statuses[]) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Waitsome(incount, array_of_requests, outcount, array_of_indices, array_of_statuses);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Waitsome =============== */
static void MPI_Waitsome_fortran_wrapper(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Waitsome(*incount, (MPI_Request*)array_of_requests, (int*)outcount, (int*)array_of_indices, (MPI_Status*)array_of_statuses);
#else /* MPI-2 safe call */
    MPI_Status* temp_array_of_statuses;
    MPI_Request* temp_array_of_requests;
    int i;
    temp_array_of_requests = (MPI_Request*)malloc(sizeof(MPI_Request) * *incount);
    for (i=0; i < *incount; i++)
        temp_array_of_requests[i] = MPI_Request_f2c(array_of_requests[i]);
    temp_array_of_statuses = (MPI_Status*)malloc(sizeof(MPI_Status) * *incount);
    for (i=0; i < *incount; i++)
        MPI_Status_f2c(&array_of_statuses[i], &temp_array_of_statuses[i]);
    _wrap_py_return_val = MPI_Waitsome(*incount, temp_array_of_requests, (int*)outcount, (int*)array_of_indices, temp_array_of_statuses);
    for (i=0; i < *incount; i++)
        array_of_requests[i] = MPI_Request_c2f(temp_array_of_requests[i]);
    free(temp_array_of_requests);
    for (i=0; i < *incount; i++)
        MPI_Status_c2f(&temp_array_of_statuses[i], &array_of_statuses[i]);
    free(temp_array_of_statuses);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WAITSOME(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Waitsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_waitsome(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Waitsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_waitsome_(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Waitsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

_EXTERN_C_ void mpi_waitsome__(MPI_Fint *incount, MPI_Fint array_of_requests[], MPI_Fint *outcount, MPI_Fint array_of_indices[], MPI_Fint array_of_statuses[], MPI_Fint *ierr) { 
    MPI_Waitsome_fortran_wrapper(incount, array_of_requests, outcount, array_of_indices, array_of_statuses, ierr);
}

/* ================= End Wrappers for MPI_Waitsome ================= */


/* ================== C Wrappers for MPI_Win_allocate ================== */
_EXTERN_C_ int PMPI_Win_allocate(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win);
_EXTERN_C_ int MPI_Win_allocate(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Win_allocate(size, disp_unit, info, comm, baseptr, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_allocate =============== */
static void MPI_Win_allocate_fortran_wrapper(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_allocate(*size, *disp_unit, (MPI_Info)(*info), (MPI_Comm)(*comm), (void*)baseptr, (MPI_Win*)win);
#else /* MPI-2 safe call */
    MPI_Win temp_win;
    temp_win = MPI_Win_f2c(*win);
    _wrap_py_return_val = MPI_Win_allocate(*size, *disp_unit, MPI_Info_f2c(*info), MPI_Comm_f2c(*comm), (void*)baseptr, &temp_win);
    *win = MPI_Win_c2f(temp_win);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_ALLOCATE(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

_EXTERN_C_ void mpi_win_allocate(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

_EXTERN_C_ void mpi_win_allocate_(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

_EXTERN_C_ void mpi_win_allocate__(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

/* ================= End Wrappers for MPI_Win_allocate ================= */


/* ================== C Wrappers for MPI_Win_allocate_shared ================== */
_EXTERN_C_ int PMPI_Win_allocate_shared(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win);
_EXTERN_C_ int MPI_Win_allocate_shared(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Win_allocate_shared(size, disp_unit, info, comm, baseptr, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_allocate_shared =============== */
static void MPI_Win_allocate_shared_fortran_wrapper(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_allocate_shared(*size, *disp_unit, (MPI_Info)(*info), (MPI_Comm)(*comm), (void*)baseptr, (MPI_Win*)win);
#else /* MPI-2 safe call */
    MPI_Win temp_win;
    temp_win = MPI_Win_f2c(*win);
    _wrap_py_return_val = MPI_Win_allocate_shared(*size, *disp_unit, MPI_Info_f2c(*info), MPI_Comm_f2c(*comm), (void*)baseptr, &temp_win);
    *win = MPI_Win_c2f(temp_win);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_ALLOCATE_SHARED(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_shared_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

_EXTERN_C_ void mpi_win_allocate_shared(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_shared_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

_EXTERN_C_ void mpi_win_allocate_shared_(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_shared_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

_EXTERN_C_ void mpi_win_allocate_shared__(MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *baseptr, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_allocate_shared_fortran_wrapper(size, disp_unit, info, comm, baseptr, win, ierr);
}

/* ================= End Wrappers for MPI_Win_allocate_shared ================= */


/* ================== C Wrappers for MPI_Win_attach ================== */
_EXTERN_C_ int PMPI_Win_attach(MPI_Win win, void *base, MPI_Aint size);
_EXTERN_C_ int MPI_Win_attach(MPI_Win win, void *base, MPI_Aint size) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_attach(win, base, size);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_attach =============== */
static void MPI_Win_attach_fortran_wrapper(MPI_Fint *win, MPI_Fint *base, MPI_Aint *size, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_attach((MPI_Win)(*win), (void*)base, *size);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_attach(MPI_Win_f2c(*win), (void*)base, *size);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_ATTACH(MPI_Fint *win, MPI_Fint *base, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Win_attach_fortran_wrapper(win, base, size, ierr);
}

_EXTERN_C_ void mpi_win_attach(MPI_Fint *win, MPI_Fint *base, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Win_attach_fortran_wrapper(win, base, size, ierr);
}

_EXTERN_C_ void mpi_win_attach_(MPI_Fint *win, MPI_Fint *base, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Win_attach_fortran_wrapper(win, base, size, ierr);
}

_EXTERN_C_ void mpi_win_attach__(MPI_Fint *win, MPI_Fint *base, MPI_Aint *size, MPI_Fint *ierr) { 
    MPI_Win_attach_fortran_wrapper(win, base, size, ierr);
}

/* ================= End Wrappers for MPI_Win_attach ================= */


/* ================== C Wrappers for MPI_Win_call_errhandler ================== */
_EXTERN_C_ int PMPI_Win_call_errhandler(MPI_Win win, int errorcode);
_EXTERN_C_ int MPI_Win_call_errhandler(MPI_Win win, int errorcode) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_call_errhandler(win, errorcode);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_call_errhandler =============== */
static void MPI_Win_call_errhandler_fortran_wrapper(MPI_Fint *win, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_call_errhandler((MPI_Win)(*win), *errorcode);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_call_errhandler(MPI_Win_f2c(*win), *errorcode);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_CALL_ERRHANDLER(MPI_Fint *win, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Win_call_errhandler_fortran_wrapper(win, errorcode, ierr);
}

_EXTERN_C_ void mpi_win_call_errhandler(MPI_Fint *win, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Win_call_errhandler_fortran_wrapper(win, errorcode, ierr);
}

_EXTERN_C_ void mpi_win_call_errhandler_(MPI_Fint *win, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Win_call_errhandler_fortran_wrapper(win, errorcode, ierr);
}

_EXTERN_C_ void mpi_win_call_errhandler__(MPI_Fint *win, MPI_Fint *errorcode, MPI_Fint *ierr) { 
    MPI_Win_call_errhandler_fortran_wrapper(win, errorcode, ierr);
}

/* ================= End Wrappers for MPI_Win_call_errhandler ================= */


/* ================== C Wrappers for MPI_Win_complete ================== */
_EXTERN_C_ int PMPI_Win_complete(MPI_Win win);
_EXTERN_C_ int MPI_Win_complete(MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_complete(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_complete =============== */
static void MPI_Win_complete_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_complete((MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_complete(MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_COMPLETE(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_complete_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_complete(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_complete_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_complete_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_complete_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_complete__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_complete_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_complete ================= */


/* ================== C Wrappers for MPI_Win_create ================== */
_EXTERN_C_ int PMPI_Win_create(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win);
_EXTERN_C_ int MPI_Win_create(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Win_create(base, size, disp_unit, info, comm, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_create =============== */
static void MPI_Win_create_fortran_wrapper(MPI_Fint *base, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_create((void*)base, *size, *disp_unit, (MPI_Info)(*info), (MPI_Comm)(*comm), (MPI_Win*)win);
#else /* MPI-2 safe call */
    MPI_Win temp_win;
    temp_win = MPI_Win_f2c(*win);
    _wrap_py_return_val = MPI_Win_create((void*)base, *size, *disp_unit, MPI_Info_f2c(*info), MPI_Comm_f2c(*comm), &temp_win);
    *win = MPI_Win_c2f(temp_win);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_CREATE(MPI_Fint *base, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_fortran_wrapper(base, size, disp_unit, info, comm, win, ierr);
}

_EXTERN_C_ void mpi_win_create(MPI_Fint *base, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_fortran_wrapper(base, size, disp_unit, info, comm, win, ierr);
}

_EXTERN_C_ void mpi_win_create_(MPI_Fint *base, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_fortran_wrapper(base, size, disp_unit, info, comm, win, ierr);
}

_EXTERN_C_ void mpi_win_create__(MPI_Fint *base, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_fortran_wrapper(base, size, disp_unit, info, comm, win, ierr);
}

/* ================= End Wrappers for MPI_Win_create ================= */


/* ================== C Wrappers for MPI_Win_create_dynamic ================== */
_EXTERN_C_ int PMPI_Win_create_dynamic(MPI_Info info, MPI_Comm comm, MPI_Win *win);
_EXTERN_C_ int MPI_Win_create_dynamic(MPI_Info info, MPI_Comm comm, MPI_Win *win) { 
    int _wrap_py_return_val = 0;
{
   swap_world(comm);

   _wrap_py_return_val = PMPI_Win_create_dynamic(info, comm, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_create_dynamic =============== */
static void MPI_Win_create_dynamic_fortran_wrapper(MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_create_dynamic((MPI_Info)(*info), (MPI_Comm)(*comm), (MPI_Win*)win);
#else /* MPI-2 safe call */
    MPI_Win temp_win;
    temp_win = MPI_Win_f2c(*win);
    _wrap_py_return_val = MPI_Win_create_dynamic(MPI_Info_f2c(*info), MPI_Comm_f2c(*comm), &temp_win);
    *win = MPI_Win_c2f(temp_win);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_CREATE_DYNAMIC(MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_dynamic_fortran_wrapper(info, comm, win, ierr);
}

_EXTERN_C_ void mpi_win_create_dynamic(MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_dynamic_fortran_wrapper(info, comm, win, ierr);
}

_EXTERN_C_ void mpi_win_create_dynamic_(MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_dynamic_fortran_wrapper(info, comm, win, ierr);
}

_EXTERN_C_ void mpi_win_create_dynamic__(MPI_Fint *info, MPI_Fint *comm, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_create_dynamic_fortran_wrapper(info, comm, win, ierr);
}

/* ================= End Wrappers for MPI_Win_create_dynamic ================= */


/* ================== C Wrappers for MPI_Win_create_errhandler ================== */
_EXTERN_C_ int PMPI_Win_create_errhandler(MPI_Win_errhandler_function *function, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_Win_create_errhandler(MPI_Win_errhandler_function *function, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_create_errhandler(function, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_create_errhandler =============== */
static void MPI_Win_create_errhandler_fortran_wrapper(MPI_Win_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_create_errhandler((MPI_Win_errhandler_function*)function, (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_Win_create_errhandler((MPI_Win_errhandler_function*)function, &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_CREATE_ERRHANDLER(MPI_Win_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_create_errhandler(MPI_Win_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_create_errhandler_(MPI_Win_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_create_errhandler__(MPI_Win_errhandler_function *function, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_create_errhandler_fortran_wrapper(function, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Win_create_errhandler ================= */


/* ================== C Wrappers for MPI_Win_create_keyval ================== */
_EXTERN_C_ int PMPI_Win_create_keyval(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, int *win_keyval, void *extra_state);
_EXTERN_C_ int MPI_Win_create_keyval(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, int *win_keyval, void *extra_state) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_create_keyval(win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_create_keyval =============== */
static void MPI_Win_create_keyval_fortran_wrapper(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, MPI_Fint *win_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Win_create_keyval((MPI_Win_copy_attr_function*)win_copy_attr_fn, (MPI_Win_delete_attr_function*)win_delete_attr_fn, (int*)win_keyval, (void*)extra_state);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_CREATE_KEYVAL(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, MPI_Fint *win_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Win_create_keyval_fortran_wrapper(win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_win_create_keyval(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, MPI_Fint *win_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Win_create_keyval_fortran_wrapper(win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_win_create_keyval_(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, MPI_Fint *win_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Win_create_keyval_fortran_wrapper(win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state, ierr);
}

_EXTERN_C_ void mpi_win_create_keyval__(MPI_Win_copy_attr_function *win_copy_attr_fn, MPI_Win_delete_attr_function *win_delete_attr_fn, MPI_Fint *win_keyval, MPI_Fint *extra_state, MPI_Fint *ierr) { 
    MPI_Win_create_keyval_fortran_wrapper(win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state, ierr);
}

/* ================= End Wrappers for MPI_Win_create_keyval ================= */


/* ================== C Wrappers for MPI_Win_delete_attr ================== */
_EXTERN_C_ int PMPI_Win_delete_attr(MPI_Win win, int win_keyval);
_EXTERN_C_ int MPI_Win_delete_attr(MPI_Win win, int win_keyval) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_delete_attr(win, win_keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_delete_attr =============== */
static void MPI_Win_delete_attr_fortran_wrapper(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_delete_attr((MPI_Win)(*win), *win_keyval);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_delete_attr(MPI_Win_f2c(*win), *win_keyval);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_DELETE_ATTR(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_delete_attr_fortran_wrapper(win, win_keyval, ierr);
}

_EXTERN_C_ void mpi_win_delete_attr(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_delete_attr_fortran_wrapper(win, win_keyval, ierr);
}

_EXTERN_C_ void mpi_win_delete_attr_(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_delete_attr_fortran_wrapper(win, win_keyval, ierr);
}

_EXTERN_C_ void mpi_win_delete_attr__(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_delete_attr_fortran_wrapper(win, win_keyval, ierr);
}

/* ================= End Wrappers for MPI_Win_delete_attr ================= */


/* ================== C Wrappers for MPI_Win_detach ================== */
_EXTERN_C_ int PMPI_Win_detach(MPI_Win win, const void *base);
_EXTERN_C_ int MPI_Win_detach(MPI_Win win, const void *base) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_detach(win, base);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_detach =============== */
static void MPI_Win_detach_fortran_wrapper(MPI_Fint *win, MPI_Fint *base, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_detach((MPI_Win)(*win), (const void*)base);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_detach(MPI_Win_f2c(*win), (const void*)base);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_DETACH(MPI_Fint *win, MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Win_detach_fortran_wrapper(win, base, ierr);
}

_EXTERN_C_ void mpi_win_detach(MPI_Fint *win, MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Win_detach_fortran_wrapper(win, base, ierr);
}

_EXTERN_C_ void mpi_win_detach_(MPI_Fint *win, MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Win_detach_fortran_wrapper(win, base, ierr);
}

_EXTERN_C_ void mpi_win_detach__(MPI_Fint *win, MPI_Fint *base, MPI_Fint *ierr) { 
    MPI_Win_detach_fortran_wrapper(win, base, ierr);
}

/* ================= End Wrappers for MPI_Win_detach ================= */


/* ================== C Wrappers for MPI_Win_fence ================== */
_EXTERN_C_ int PMPI_Win_fence(int assert, MPI_Win win);
_EXTERN_C_ int MPI_Win_fence(int assert, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_fence(assert, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_fence =============== */
static void MPI_Win_fence_fortran_wrapper(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_fence(*assert, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_fence(*assert, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FENCE(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_fence_fortran_wrapper(assert, win, ierr);
}

_EXTERN_C_ void mpi_win_fence(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_fence_fortran_wrapper(assert, win, ierr);
}

_EXTERN_C_ void mpi_win_fence_(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_fence_fortran_wrapper(assert, win, ierr);
}

_EXTERN_C_ void mpi_win_fence__(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_fence_fortran_wrapper(assert, win, ierr);
}

/* ================= End Wrappers for MPI_Win_fence ================= */


/* ================== C Wrappers for MPI_Win_flush ================== */
_EXTERN_C_ int PMPI_Win_flush(int rank, MPI_Win win);
_EXTERN_C_ int MPI_Win_flush(int rank, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_flush(rank, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_flush =============== */
static void MPI_Win_flush_fortran_wrapper(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_flush(*rank, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_flush(*rank, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FLUSH(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_flush(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_flush_(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_flush__(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_fortran_wrapper(rank, win, ierr);
}

/* ================= End Wrappers for MPI_Win_flush ================= */


/* ================== C Wrappers for MPI_Win_flush_all ================== */
_EXTERN_C_ int PMPI_Win_flush_all(MPI_Win win);
_EXTERN_C_ int MPI_Win_flush_all(MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_flush_all(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_flush_all =============== */
static void MPI_Win_flush_all_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_flush_all((MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_flush_all(MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FLUSH_ALL(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_flush_all(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_flush_all_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_flush_all__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_all_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_flush_all ================= */


/* ================== C Wrappers for MPI_Win_flush_local ================== */
_EXTERN_C_ int PMPI_Win_flush_local(int rank, MPI_Win win);
_EXTERN_C_ int MPI_Win_flush_local(int rank, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_flush_local(rank, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_flush_local =============== */
static void MPI_Win_flush_local_fortran_wrapper(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_flush_local(*rank, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_flush_local(*rank, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FLUSH_LOCAL(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_flush_local(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_flush_local_(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_flush_local__(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_fortran_wrapper(rank, win, ierr);
}

/* ================= End Wrappers for MPI_Win_flush_local ================= */


/* ================== C Wrappers for MPI_Win_flush_local_all ================== */
_EXTERN_C_ int PMPI_Win_flush_local_all(MPI_Win win);
_EXTERN_C_ int MPI_Win_flush_local_all(MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_flush_local_all(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_flush_local_all =============== */
static void MPI_Win_flush_local_all_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_flush_local_all((MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_flush_local_all(MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FLUSH_LOCAL_ALL(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_flush_local_all(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_flush_local_all_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_flush_local_all__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_flush_local_all_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_flush_local_all ================= */


/* ================== C Wrappers for MPI_Win_free ================== */
_EXTERN_C_ int PMPI_Win_free(MPI_Win *win);
_EXTERN_C_ int MPI_Win_free(MPI_Win *win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_free(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_free =============== */
static void MPI_Win_free_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_free((MPI_Win*)win);
#else /* MPI-2 safe call */
    MPI_Win temp_win;
    temp_win = MPI_Win_f2c(*win);
    _wrap_py_return_val = MPI_Win_free(&temp_win);
    *win = MPI_Win_c2f(temp_win);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FREE(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_free_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_free(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_free_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_free_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_free_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_free__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_free_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_free ================= */


/* ================== C Wrappers for MPI_Win_free_keyval ================== */
_EXTERN_C_ int PMPI_Win_free_keyval(int *win_keyval);
_EXTERN_C_ int MPI_Win_free_keyval(int *win_keyval) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_free_keyval(win_keyval);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_free_keyval =============== */
static void MPI_Win_free_keyval_fortran_wrapper(MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Win_free_keyval((int*)win_keyval);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_FREE_KEYVAL(MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_free_keyval_fortran_wrapper(win_keyval, ierr);
}

_EXTERN_C_ void mpi_win_free_keyval(MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_free_keyval_fortran_wrapper(win_keyval, ierr);
}

_EXTERN_C_ void mpi_win_free_keyval_(MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_free_keyval_fortran_wrapper(win_keyval, ierr);
}

_EXTERN_C_ void mpi_win_free_keyval__(MPI_Fint *win_keyval, MPI_Fint *ierr) { 
    MPI_Win_free_keyval_fortran_wrapper(win_keyval, ierr);
}

/* ================= End Wrappers for MPI_Win_free_keyval ================= */


/* ================== C Wrappers for MPI_Win_get_attr ================== */
_EXTERN_C_ int PMPI_Win_get_attr(MPI_Win win, int win_keyval, void *attribute_val, int *flag);
_EXTERN_C_ int MPI_Win_get_attr(MPI_Win win, int win_keyval, void *attribute_val, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_get_attr(win, win_keyval, attribute_val, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_get_attr =============== */
static void MPI_Win_get_attr_fortran_wrapper(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_get_attr((MPI_Win)(*win), *win_keyval, (void*)attribute_val, (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_get_attr(MPI_Win_f2c(*win), *win_keyval, (void*)attribute_val, (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_GET_ATTR(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_get_attr_fortran_wrapper(win, win_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_win_get_attr(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_get_attr_fortran_wrapper(win, win_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_win_get_attr_(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_get_attr_fortran_wrapper(win, win_keyval, attribute_val, flag, ierr);
}

_EXTERN_C_ void mpi_win_get_attr__(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_get_attr_fortran_wrapper(win, win_keyval, attribute_val, flag, ierr);
}

/* ================= End Wrappers for MPI_Win_get_attr ================= */


/* ================== C Wrappers for MPI_Win_get_errhandler ================== */
_EXTERN_C_ int PMPI_Win_get_errhandler(MPI_Win win, MPI_Errhandler *errhandler);
_EXTERN_C_ int MPI_Win_get_errhandler(MPI_Win win, MPI_Errhandler *errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_get_errhandler(win, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_get_errhandler =============== */
static void MPI_Win_get_errhandler_fortran_wrapper(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_get_errhandler((MPI_Win)(*win), (MPI_Errhandler*)errhandler);
#else /* MPI-2 safe call */
    MPI_Errhandler temp_errhandler;
    temp_errhandler = MPI_Errhandler_f2c(*errhandler);
    _wrap_py_return_val = MPI_Win_get_errhandler(MPI_Win_f2c(*win), &temp_errhandler);
    *errhandler = MPI_Errhandler_c2f(temp_errhandler);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_GET_ERRHANDLER(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_get_errhandler_fortran_wrapper(win, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_get_errhandler(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_get_errhandler_fortran_wrapper(win, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_get_errhandler_(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_get_errhandler_fortran_wrapper(win, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_get_errhandler__(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_get_errhandler_fortran_wrapper(win, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Win_get_errhandler ================= */


/* ================== C Wrappers for MPI_Win_get_group ================== */
_EXTERN_C_ int PMPI_Win_get_group(MPI_Win win, MPI_Group *group);
_EXTERN_C_ int MPI_Win_get_group(MPI_Win win, MPI_Group *group) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_get_group(win, group);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_get_group =============== */
static void MPI_Win_get_group_fortran_wrapper(MPI_Fint *win, MPI_Fint *group, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_get_group((MPI_Win)(*win), (MPI_Group*)group);
#else /* MPI-2 safe call */
    MPI_Group temp_group;
    temp_group = MPI_Group_f2c(*group);
    _wrap_py_return_val = MPI_Win_get_group(MPI_Win_f2c(*win), &temp_group);
    *group = MPI_Group_c2f(temp_group);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_GET_GROUP(MPI_Fint *win, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Win_get_group_fortran_wrapper(win, group, ierr);
}

_EXTERN_C_ void mpi_win_get_group(MPI_Fint *win, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Win_get_group_fortran_wrapper(win, group, ierr);
}

_EXTERN_C_ void mpi_win_get_group_(MPI_Fint *win, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Win_get_group_fortran_wrapper(win, group, ierr);
}

_EXTERN_C_ void mpi_win_get_group__(MPI_Fint *win, MPI_Fint *group, MPI_Fint *ierr) { 
    MPI_Win_get_group_fortran_wrapper(win, group, ierr);
}

/* ================= End Wrappers for MPI_Win_get_group ================= */


/* ================== C Wrappers for MPI_Win_get_info ================== */
_EXTERN_C_ int PMPI_Win_get_info(MPI_Win win, MPI_Info *info_used);
_EXTERN_C_ int MPI_Win_get_info(MPI_Win win, MPI_Info *info_used) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_get_info(win, info_used);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_get_info =============== */
static void MPI_Win_get_info_fortran_wrapper(MPI_Fint *win, MPI_Fint *info_used, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_get_info((MPI_Win)(*win), (MPI_Info*)info_used);
#else /* MPI-2 safe call */
    MPI_Info temp_info_used;
    temp_info_used = MPI_Info_f2c(*info_used);
    _wrap_py_return_val = MPI_Win_get_info(MPI_Win_f2c(*win), &temp_info_used);
    *info_used = MPI_Info_c2f(temp_info_used);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_GET_INFO(MPI_Fint *win, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Win_get_info_fortran_wrapper(win, info_used, ierr);
}

_EXTERN_C_ void mpi_win_get_info(MPI_Fint *win, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Win_get_info_fortran_wrapper(win, info_used, ierr);
}

_EXTERN_C_ void mpi_win_get_info_(MPI_Fint *win, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Win_get_info_fortran_wrapper(win, info_used, ierr);
}

_EXTERN_C_ void mpi_win_get_info__(MPI_Fint *win, MPI_Fint *info_used, MPI_Fint *ierr) { 
    MPI_Win_get_info_fortran_wrapper(win, info_used, ierr);
}

/* ================= End Wrappers for MPI_Win_get_info ================= */


/* ================== C Wrappers for MPI_Win_get_name ================== */
_EXTERN_C_ int PMPI_Win_get_name(MPI_Win win, char *win_name, int *resultlen);
_EXTERN_C_ int MPI_Win_get_name(MPI_Win win, char *win_name, int *resultlen) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_get_name(win, win_name, resultlen);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_get_name =============== */
static void MPI_Win_get_name_fortran_wrapper(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_get_name((MPI_Win)(*win), (char*)win_name, (int*)resultlen);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_get_name(MPI_Win_f2c(*win), (char*)win_name, (int*)resultlen);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_GET_NAME(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Win_get_name_fortran_wrapper(win, win_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_win_get_name(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Win_get_name_fortran_wrapper(win, win_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_win_get_name_(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Win_get_name_fortran_wrapper(win, win_name, resultlen, ierr);
}

_EXTERN_C_ void mpi_win_get_name__(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *resultlen, MPI_Fint *ierr) { 
    MPI_Win_get_name_fortran_wrapper(win, win_name, resultlen, ierr);
}

/* ================= End Wrappers for MPI_Win_get_name ================= */


/* ================== C Wrappers for MPI_Win_lock ================== */
_EXTERN_C_ int PMPI_Win_lock(int lock_type, int rank, int assert, MPI_Win win);
_EXTERN_C_ int MPI_Win_lock(int lock_type, int rank, int assert, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_lock(lock_type, rank, assert, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_lock =============== */
static void MPI_Win_lock_fortran_wrapper(MPI_Fint *lock_type, MPI_Fint *rank, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_lock(*lock_type, *rank, *assert, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_lock(*lock_type, *rank, *assert, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_LOCK(MPI_Fint *lock_type, MPI_Fint *rank, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_fortran_wrapper(lock_type, rank, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_lock(MPI_Fint *lock_type, MPI_Fint *rank, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_fortran_wrapper(lock_type, rank, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_lock_(MPI_Fint *lock_type, MPI_Fint *rank, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_fortran_wrapper(lock_type, rank, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_lock__(MPI_Fint *lock_type, MPI_Fint *rank, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_fortran_wrapper(lock_type, rank, assert, win, ierr);
}

/* ================= End Wrappers for MPI_Win_lock ================= */


/* ================== C Wrappers for MPI_Win_lock_all ================== */
_EXTERN_C_ int PMPI_Win_lock_all(int assert, MPI_Win win);
_EXTERN_C_ int MPI_Win_lock_all(int assert, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_lock_all(assert, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_lock_all =============== */
static void MPI_Win_lock_all_fortran_wrapper(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_lock_all(*assert, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_lock_all(*assert, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_LOCK_ALL(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_all_fortran_wrapper(assert, win, ierr);
}

_EXTERN_C_ void mpi_win_lock_all(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_all_fortran_wrapper(assert, win, ierr);
}

_EXTERN_C_ void mpi_win_lock_all_(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_all_fortran_wrapper(assert, win, ierr);
}

_EXTERN_C_ void mpi_win_lock_all__(MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_lock_all_fortran_wrapper(assert, win, ierr);
}

/* ================= End Wrappers for MPI_Win_lock_all ================= */


/* ================== C Wrappers for MPI_Win_post ================== */
_EXTERN_C_ int PMPI_Win_post(MPI_Group group, int assert, MPI_Win win);
_EXTERN_C_ int MPI_Win_post(MPI_Group group, int assert, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_post(group, assert, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_post =============== */
static void MPI_Win_post_fortran_wrapper(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_post((MPI_Group)(*group), *assert, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_post(MPI_Group_f2c(*group), *assert, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_POST(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_post_fortran_wrapper(group, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_post(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_post_fortran_wrapper(group, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_post_(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_post_fortran_wrapper(group, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_post__(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_post_fortran_wrapper(group, assert, win, ierr);
}

/* ================= End Wrappers for MPI_Win_post ================= */


/* ================== C Wrappers for MPI_Win_set_attr ================== */
_EXTERN_C_ int PMPI_Win_set_attr(MPI_Win win, int win_keyval, void *attribute_val);
_EXTERN_C_ int MPI_Win_set_attr(MPI_Win win, int win_keyval, void *attribute_val) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_set_attr(win, win_keyval, attribute_val);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_set_attr =============== */
static void MPI_Win_set_attr_fortran_wrapper(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_set_attr((MPI_Win)(*win), *win_keyval, (void*)attribute_val);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_set_attr(MPI_Win_f2c(*win), *win_keyval, (void*)attribute_val);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_SET_ATTR(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Win_set_attr_fortran_wrapper(win, win_keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_win_set_attr(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Win_set_attr_fortran_wrapper(win, win_keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_win_set_attr_(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Win_set_attr_fortran_wrapper(win, win_keyval, attribute_val, ierr);
}

_EXTERN_C_ void mpi_win_set_attr__(MPI_Fint *win, MPI_Fint *win_keyval, MPI_Fint *attribute_val, MPI_Fint *ierr) { 
    MPI_Win_set_attr_fortran_wrapper(win, win_keyval, attribute_val, ierr);
}

/* ================= End Wrappers for MPI_Win_set_attr ================= */


/* ================== C Wrappers for MPI_Win_set_errhandler ================== */
_EXTERN_C_ int PMPI_Win_set_errhandler(MPI_Win win, MPI_Errhandler errhandler);
_EXTERN_C_ int MPI_Win_set_errhandler(MPI_Win win, MPI_Errhandler errhandler) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_set_errhandler(win, errhandler);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_set_errhandler =============== */
static void MPI_Win_set_errhandler_fortran_wrapper(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_set_errhandler((MPI_Win)(*win), (MPI_Errhandler)(*errhandler));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_set_errhandler(MPI_Win_f2c(*win), MPI_Errhandler_f2c(*errhandler));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_SET_ERRHANDLER(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_set_errhandler_fortran_wrapper(win, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_set_errhandler(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_set_errhandler_fortran_wrapper(win, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_set_errhandler_(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_set_errhandler_fortran_wrapper(win, errhandler, ierr);
}

_EXTERN_C_ void mpi_win_set_errhandler__(MPI_Fint *win, MPI_Fint *errhandler, MPI_Fint *ierr) { 
    MPI_Win_set_errhandler_fortran_wrapper(win, errhandler, ierr);
}

/* ================= End Wrappers for MPI_Win_set_errhandler ================= */


/* ================== C Wrappers for MPI_Win_set_info ================== */
_EXTERN_C_ int PMPI_Win_set_info(MPI_Win win, MPI_Info info);
_EXTERN_C_ int MPI_Win_set_info(MPI_Win win, MPI_Info info) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_set_info(win, info);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_set_info =============== */
static void MPI_Win_set_info_fortran_wrapper(MPI_Fint *win, MPI_Fint *info, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_set_info((MPI_Win)(*win), (MPI_Info)(*info));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_set_info(MPI_Win_f2c(*win), MPI_Info_f2c(*info));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_SET_INFO(MPI_Fint *win, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Win_set_info_fortran_wrapper(win, info, ierr);
}

_EXTERN_C_ void mpi_win_set_info(MPI_Fint *win, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Win_set_info_fortran_wrapper(win, info, ierr);
}

_EXTERN_C_ void mpi_win_set_info_(MPI_Fint *win, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Win_set_info_fortran_wrapper(win, info, ierr);
}

_EXTERN_C_ void mpi_win_set_info__(MPI_Fint *win, MPI_Fint *info, MPI_Fint *ierr) { 
    MPI_Win_set_info_fortran_wrapper(win, info, ierr);
}

/* ================= End Wrappers for MPI_Win_set_info ================= */


/* ================== C Wrappers for MPI_Win_set_name ================== */
_EXTERN_C_ int PMPI_Win_set_name(MPI_Win win, const char *win_name);
_EXTERN_C_ int MPI_Win_set_name(MPI_Win win, const char *win_name) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_set_name(win, win_name);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_set_name =============== */
static void MPI_Win_set_name_fortran_wrapper(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_set_name((MPI_Win)(*win), (const char*)win_name);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_set_name(MPI_Win_f2c(*win), (const char*)win_name);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_SET_NAME(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *ierr) { 
    MPI_Win_set_name_fortran_wrapper(win, win_name, ierr);
}

_EXTERN_C_ void mpi_win_set_name(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *ierr) { 
    MPI_Win_set_name_fortran_wrapper(win, win_name, ierr);
}

_EXTERN_C_ void mpi_win_set_name_(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *ierr) { 
    MPI_Win_set_name_fortran_wrapper(win, win_name, ierr);
}

_EXTERN_C_ void mpi_win_set_name__(MPI_Fint *win, MPI_Fint *win_name, MPI_Fint *ierr) { 
    MPI_Win_set_name_fortran_wrapper(win, win_name, ierr);
}

/* ================= End Wrappers for MPI_Win_set_name ================= */


/* ================== C Wrappers for MPI_Win_shared_query ================== */
_EXTERN_C_ int PMPI_Win_shared_query(MPI_Win win, int rank, MPI_Aint *size, int *disp_unit, void *baseptr);
_EXTERN_C_ int MPI_Win_shared_query(MPI_Win win, int rank, MPI_Aint *size, int *disp_unit, void *baseptr) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_shared_query(win, rank, size, disp_unit, baseptr);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_shared_query =============== */
static void MPI_Win_shared_query_fortran_wrapper(MPI_Fint *win, MPI_Fint *rank, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_shared_query((MPI_Win)(*win), *rank, (MPI_Aint*)size, (int*)disp_unit, (void*)baseptr);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_shared_query(MPI_Win_f2c(*win), *rank, (MPI_Aint*)size, (int*)disp_unit, (void*)baseptr);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_SHARED_QUERY(MPI_Fint *win, MPI_Fint *rank, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Win_shared_query_fortran_wrapper(win, rank, size, disp_unit, baseptr, ierr);
}

_EXTERN_C_ void mpi_win_shared_query(MPI_Fint *win, MPI_Fint *rank, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Win_shared_query_fortran_wrapper(win, rank, size, disp_unit, baseptr, ierr);
}

_EXTERN_C_ void mpi_win_shared_query_(MPI_Fint *win, MPI_Fint *rank, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Win_shared_query_fortran_wrapper(win, rank, size, disp_unit, baseptr, ierr);
}

_EXTERN_C_ void mpi_win_shared_query__(MPI_Fint *win, MPI_Fint *rank, MPI_Aint *size, MPI_Fint *disp_unit, MPI_Fint *baseptr, MPI_Fint *ierr) { 
    MPI_Win_shared_query_fortran_wrapper(win, rank, size, disp_unit, baseptr, ierr);
}

/* ================= End Wrappers for MPI_Win_shared_query ================= */


/* ================== C Wrappers for MPI_Win_start ================== */
_EXTERN_C_ int PMPI_Win_start(MPI_Group group, int assert, MPI_Win win);
_EXTERN_C_ int MPI_Win_start(MPI_Group group, int assert, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_start(group, assert, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_start =============== */
static void MPI_Win_start_fortran_wrapper(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_start((MPI_Group)(*group), *assert, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_start(MPI_Group_f2c(*group), *assert, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_START(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_start_fortran_wrapper(group, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_start(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_start_fortran_wrapper(group, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_start_(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_start_fortran_wrapper(group, assert, win, ierr);
}

_EXTERN_C_ void mpi_win_start__(MPI_Fint *group, MPI_Fint *assert, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_start_fortran_wrapper(group, assert, win, ierr);
}

/* ================= End Wrappers for MPI_Win_start ================= */


/* ================== C Wrappers for MPI_Win_sync ================== */
_EXTERN_C_ int PMPI_Win_sync(MPI_Win win);
_EXTERN_C_ int MPI_Win_sync(MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_sync(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_sync =============== */
static void MPI_Win_sync_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_sync((MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_sync(MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_SYNC(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_sync_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_sync(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_sync_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_sync_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_sync_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_sync__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_sync_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_sync ================= */


/* ================== C Wrappers for MPI_Win_test ================== */
_EXTERN_C_ int PMPI_Win_test(MPI_Win win, int *flag);
_EXTERN_C_ int MPI_Win_test(MPI_Win win, int *flag) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_test(win, flag);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_test =============== */
static void MPI_Win_test_fortran_wrapper(MPI_Fint *win, MPI_Fint *flag, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_test((MPI_Win)(*win), (int*)flag);
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_test(MPI_Win_f2c(*win), (int*)flag);
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_TEST(MPI_Fint *win, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_test_fortran_wrapper(win, flag, ierr);
}

_EXTERN_C_ void mpi_win_test(MPI_Fint *win, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_test_fortran_wrapper(win, flag, ierr);
}

_EXTERN_C_ void mpi_win_test_(MPI_Fint *win, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_test_fortran_wrapper(win, flag, ierr);
}

_EXTERN_C_ void mpi_win_test__(MPI_Fint *win, MPI_Fint *flag, MPI_Fint *ierr) { 
    MPI_Win_test_fortran_wrapper(win, flag, ierr);
}

/* ================= End Wrappers for MPI_Win_test ================= */


/* ================== C Wrappers for MPI_Win_unlock ================== */
_EXTERN_C_ int PMPI_Win_unlock(int rank, MPI_Win win);
_EXTERN_C_ int MPI_Win_unlock(int rank, MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_unlock(rank, win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_unlock =============== */
static void MPI_Win_unlock_fortran_wrapper(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_unlock(*rank, (MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_unlock(*rank, MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_UNLOCK(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_unlock(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_unlock_(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_fortran_wrapper(rank, win, ierr);
}

_EXTERN_C_ void mpi_win_unlock__(MPI_Fint *rank, MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_fortran_wrapper(rank, win, ierr);
}

/* ================= End Wrappers for MPI_Win_unlock ================= */


/* ================== C Wrappers for MPI_Win_unlock_all ================== */
_EXTERN_C_ int PMPI_Win_unlock_all(MPI_Win win);
_EXTERN_C_ int MPI_Win_unlock_all(MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_unlock_all(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_unlock_all =============== */
static void MPI_Win_unlock_all_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_unlock_all((MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_unlock_all(MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_UNLOCK_ALL(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_unlock_all(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_unlock_all_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_all_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_unlock_all__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_unlock_all_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_unlock_all ================= */


/* ================== C Wrappers for MPI_Win_wait ================== */
_EXTERN_C_ int PMPI_Win_wait(MPI_Win win);
_EXTERN_C_ int MPI_Win_wait(MPI_Win win) { 
    int _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Win_wait(win);
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Win_wait =============== */
static void MPI_Win_wait_fortran_wrapper(MPI_Fint *win, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Win_wait((MPI_Win)(*win));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Win_wait(MPI_Win_f2c(*win));
#endif /* MPICH test */
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_WIN_WAIT(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_wait_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_wait(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_wait_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_wait_(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_wait_fortran_wrapper(win, ierr);
}

_EXTERN_C_ void mpi_win_wait__(MPI_Fint *win, MPI_Fint *ierr) { 
    MPI_Win_wait_fortran_wrapper(win, ierr);
}

/* ================= End Wrappers for MPI_Win_wait ================= */


/* ================== C Wrappers for MPI_Wtick ================== */
_EXTERN_C_ double PMPI_Wtick();
_EXTERN_C_ double MPI_Wtick() { 
    double _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Wtick();
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Wtick =============== */
static double MPI_Wtick_fortran_wrapper() { 
    double _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Wtick();
    return _wrap_py_return_val;
}

_EXTERN_C_ double MPI_WTICK() { 
    return MPI_Wtick_fortran_wrapper();
}

_EXTERN_C_ double mpi_wtick() { 
    return MPI_Wtick_fortran_wrapper();
}

_EXTERN_C_ double mpi_wtick_() { 
    return MPI_Wtick_fortran_wrapper();
}

_EXTERN_C_ double mpi_wtick__() { 
    return MPI_Wtick_fortran_wrapper();
}

/* ================= End Wrappers for MPI_Wtick ================= */


/* ================== C Wrappers for MPI_Wtime ================== */
_EXTERN_C_ double PMPI_Wtime();
_EXTERN_C_ double MPI_Wtime() { 
    double _wrap_py_return_val = 0;
{
   
   _wrap_py_return_val = PMPI_Wtime();
}    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Wtime =============== */
static double MPI_Wtime_fortran_wrapper() { 
    double _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Wtime();
    return _wrap_py_return_val;
}

_EXTERN_C_ double MPI_WTIME() { 
    return MPI_Wtime_fortran_wrapper();
}

_EXTERN_C_ double mpi_wtime() { 
    return MPI_Wtime_fortran_wrapper();
}

_EXTERN_C_ double mpi_wtime_() { 
    return MPI_Wtime_fortran_wrapper();
}

_EXTERN_C_ double mpi_wtime__() { 
    return MPI_Wtime_fortran_wrapper();
}

/* ================= End Wrappers for MPI_Wtime ================= */



