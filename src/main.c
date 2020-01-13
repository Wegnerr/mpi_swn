#include <mpi.h>
#include <stdio.h>
#include "lib/detector.h"
#include "lib/msg.h"
#include "lib/token.h"
#include <unistd.h>

/*
Compile with mpicc main.c -o main
Run 	with mpirun -np <number_of_nodes> main

*/
int main(int argc, char *argv[]) {
    int rank, size, token, dest;

    // Initialize MPI environment
    MPI_Init (&argc, &argv);

    // Get rank, size and processor name
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (size < 2) {
        fprintf(stderr, "Number of processes must be larger than 1 in order to run this example\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        token = 1;
        MPI_Send(&token, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        token = 0;
        printf("[%i] Token sent to process %i\n",
                rank, rank+1);
        MPI_Recv(&token, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sleep(1); //Print sync
        printf("[%i] Token received from process %i\n",
                rank, size-1);
    } else {
        MPI_Recv(&token, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sleep(1); //Print sync
        printf("[%i] Token received from process %i\n",
                rank, rank-1);
        if (rank == size - 1)
            dest = 0;
        else
            dest = rank + 1;
        MPI_Send(&token, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        token = 0;
        printf("[%i] Token sent to process %i\n",
                rank, dest);
    }

    // Finalize MPI environment
    MPI_Finalize();

    return 0;
}
