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
{{fn func MPI_Init MPI_Reduce MPI_Send}}{
    // First call PMPI_Init()
    {{callfn}}

    PMPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    //cout << mpiRank << "*****\n";
}{{endfn}}
