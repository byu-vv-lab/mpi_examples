#include <mpi.h>
#include <stdio.h>

int main (int argc, char **argv) {

    int rank, size;
    int sbuff, rbuff;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);

    sbuff = 100;

// requires atleast 3 process
    if(rank == 0) {
      MPI_Send(&sbuff, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
      MPI_Send(&sbuff, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
      MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      
    }
    else if (rank == 1) {
      MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      MPI_Send(&sbuff, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&sbuff, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
      MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    }
    else if (rank == 2){
      MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      MPI_Send(&sbuff, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&sbuff, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    printf("Finished successfully \n");
    return 0;
}
