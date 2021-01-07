
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
#include <iostream>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;
//int mpiRank = 0;

//To do opaque ordering
struct RequestConverter
{
      char data[sizeof(MPI_Request)];
      RequestConverter(MPI_Request * mpi_request)
      {
            memcpy(data, mpi_request, sizeof(MPI_Request));
      }
      RequestConverter()
      { }
      RequestConverter(const RequestConverter & req)
      {
            memcpy(data, req.data, sizeof(MPI_Request));
      }
      RequestConverter & operator=(const RequestConverter & req)
      {
            memcpy(data, req.data, sizeof(MPI_Request));
            return *this;
      }
      bool operator<(const RequestConverter & request) const
      {
            for(size_t i=0; i<sizeof(MPI_Request); i++)
            {
                  if(data[i]!=request.data[i])
                  {
                        return data[i]<request.data[i];
                  }
            }
            return false;
      }
};

//To store the created MPI_Request
std::map<RequestConverter, pair<int, int>> request_holder;


//
/* ================== C Wrappers for MPI_Isend ================== */
_EXTERN_C_ int PMPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
    // First call PMPI_Init()
    int ier = _wrap_py_return_val = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
          int myid;
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    request_holder[RequestConverter(request)]=pair<int, int>(myid, tag);
    return ier;
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





/* ================== C Wrappers for MPI_Irecv ================== */
_EXTERN_C_ int PMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);
_EXTERN_C_ int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) { 
    int _wrap_py_return_val = 0;
{
    // First call PMPI_Init()
    int ier = _wrap_py_return_val = PMPI_Irecv(buf, count, datatype, source, tag, comm, request);

    request_holder[RequestConverter(request)]=pair<int, int>(source, tag);
    return ier;
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





/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int PMPI_Wait(MPI_Request *request, MPI_Status *status);
_EXTERN_C_ int MPI_Wait(MPI_Request *request, MPI_Status *status) { 
    int _wrap_py_return_val = 0;
{
      int myid;
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);
      pair<int, int> p = request_holder[RequestConverter(request)];
      std::cout << "waiting("<<myid<<")-> "<<p.first << " " << p.second<<std::endl;
      request_holder.erase(RequestConverter(request));
    return _wrap_py_return_val = PMPI_Wait(request, status);;
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
      int myid;
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);
      for (int i = 0 ; i < count; i ++){
          pair<int, int> p = request_holder[RequestConverter(&array_of_requests[i])];
        std::cout << "waiting("<<myid<<")-> "<<p.first << " " << p.second<<std::endl;
        request_holder.erase(RequestConverter(&array_of_requests[i]));
      }
    return _wrap_py_return_val = PMPI_Waitall(count, array_of_requests, array_of_statuses);;
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


