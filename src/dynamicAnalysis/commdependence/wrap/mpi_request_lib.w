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
{{fn func MPI_Isend}}{
    // First call PMPI_Init()
    int ier = {{callfn}}
          int myid;
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    request_holder[RequestConverter(request)]=pair<int, int>(myid, tag);
    return ier;
}{{endfn}}


{{fn func MPI_Irecv}}{
    // First call PMPI_Init()
    int ier = {{callfn}}

    request_holder[RequestConverter(request)]=pair<int, int>(source, tag);
    return ier;
}{{endfn}}


{{fn func MPI_Wait}}{
      int myid;
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);
      pair<int, int> p = request_holder[RequestConverter(request)];
      std::cout << "waiting("<<myid<<")-> "<<p.first << " " << p.second<<std::endl;
      request_holder.erase(RequestConverter(request));
    return {{callfn}};
}{{endfn}}


{{fn func MPI_Waitall}}{
      int myid;
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);
      for (int i = 0 ; i < count; i ++){
          pair<int, int> p = request_holder[RequestConverter(&array_of_requests[i])];
        std::cout << "waiting("<<myid<<")-> "<<p.first << " " << p.second<<std::endl;
        request_holder.erase(RequestConverter(&array_of_requests[i]));
      }
    return {{callfn}};
}{{endfn}}