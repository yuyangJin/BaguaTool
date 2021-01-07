#include <mpi.h>

#include <iostream>

int main(int argv, char ** args)
{
       int myid, numprocs;
      MPI_Init(&argv, &args);
      MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
      MPI_Comm_rank(MPI_COMM_WORLD, &myid);

      int i=123456789;
      MPI_Request request;
      MPI_Status status;
      if(myid==0)
      {
            MPI_Isend(&i, 1, MPI_INT, 1, 44444, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
            std::cout << myid <<' '<<i << std::endl;
      }
      else if(myid==1)
      {
            MPI_Irecv(&i, 1, MPI_INT, 0, 44444, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
            std::cout << myid <<' '<<i << std::endl;
      }
      int * sb = new int[numprocs];
      for(size_t i=0; i<numprocs; i++){sb[i]=(myid+1)*(i+1);}
      int * rb = new int[numprocs];
      MPI_Alltoall(sb, 1, MPI_INT, rb, 1, MPI_INT, MPI_COMM_WORLD  );
      MPI_Finalize();
}