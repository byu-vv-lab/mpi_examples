#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int rank, size;
    int sbuff, rbuff, i, j;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* printf("rank : %d\n", rank); */

    sbuff = 100;

    // requires atleast 3 process
    if(rank == 0) {
         MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

	 MPI_Barrier(MPI_COMM_WORLD);

         MPI_Recv(&rbuff, 1, MPI_INT, MPI_ANY_SOURCE,1, MPI_COMM_WORLD, &status);

    }
    else if(rank == 1) {
        MPI_Send(&sbuff, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

    }

    else if(rank == 2) {
	MPI_Barrier(MPI_COMM_WORLD);

        MPI_Send(&sbuff, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();


    /* if(!rank)  */
    /*     printf("rbuff : %d\n", rbuff); */



    return 0;
}
