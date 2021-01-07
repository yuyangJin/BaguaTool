/**
 * @author RookieHPC
 * @brief Original source code at https://www.rookiehpc.com/mpi/docs/mpi_comm_split.php
 **/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
/**
 * @brief Illustrates
 * @details MPI processes split into two groups depending on whether their rank
 * is even.
 *
 * +----------------+---+---+---+---+
 * | MPI processes  | 0 | 1 | 2 | 3 |
 * +----------------+---+---+---+---+
 * | MPI_COMM_WORLD | X | X | X | X |
 * | Subgroup A     | X |   | X |   |
 * | Subgroup B     |   | X |   | X |
 * +----------------+---+---+---+---+
 *
 * In subcommunicator A, MPI processes are assigned ranks in the same order as
 * their rank in the global communicator.
 * In subcommunicator B, MPI processes are assigned ranks in the opposite order
 * as their rank in the global communicator.
 **/

float integral(float ai, float h, int n)
{
      int j;
      float aij, integ;

      integ = 0.0;                 /* initialize */
      for (j=0;j<n;j++) {          /* sum integrals */
        aij = ai + (j+0.5)*h;      /* mid-point */
        integ += cos(aij)*h;
      }
      return integ;
}


int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
 
    // Check that 4 MPI processes are used
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    // if(comm_size != 4)
    // {
    //     printf("This application is meant to be run with 4 MPI processes, not %d.\n", comm_size);
    //     MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    // }
 
    // Get my rank in the global communicator
    int my_rank, n = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
    float h, integral_sum, a, b, ai, pi, my_int;
    pi = acos(-1.0);  /* = 3.14159... */
    a = 0.;           /* lower limit of integration */
    b = pi*1./2.;     /* upper limit of integration */
    if(my_rank == 0){ 
      n = 10000000;
    }
    MPI_Bcast( &n, 1, MPI_INT, 0, MPI_COMM_WORLD); 


    h = (float)(my_rank ) * 10000.0 * (b-a) / (float)(comm_size) + 10000; /* length of increment */
    //printf("%d %lf\n", my_rank, h);
    ai = a + my_rank * n * h; /* lower limit of integration for partition myid */
    my_int = integral(ai, h, n);  /* 0<=myid<=p-1 */

    printf("Process %d has the partial result of %f\n", my_rank, my_int);

    // Determine the colour and key based on whether my rank is even.
    char subcommunicator;
    int colour;
    int key;
    if(my_rank % 2 == 0)
    {
        subcommunicator = 'A';
        colour = 0;
        key = my_rank;
    }
    else
    {
        subcommunicator = 'B';
        colour = 1;
        key = comm_size - my_rank;
    }
 
    // Split de global communicator
    MPI_Comm new_comm;
    MPI_Comm_split(MPI_COMM_WORLD, colour, key, &new_comm);
 
    // Get my rank in the new communicator
    int my_new_comm_rank;
    MPI_Comm_rank(new_comm, &my_new_comm_rank);


    // Print my new rank and new communicator
    //printf("[MPI process %d] I am now MPI process %d in subcommunicator %c.\n", my_rank, my_new_comm_rank, subcommunicator);

    MPI_Reduce(   
          &my_int,      /* send buffer */
          &integral_sum, 1, MPI_FLOAT,    /* triplet of receive buffer, size, data type */
          MPI_SUM,    /* the reduction operation is summation */
          0, new_comm);

    if(my_new_comm_rank == 0) {
      printf("[MPI process %d in subcommunicator %c] The result = %f\n", my_rank, subcommunicator, integral_sum);
    }

    MPI_Finalize();

 
    return EXIT_SUCCESS;
}
