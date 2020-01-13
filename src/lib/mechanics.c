#include "mechanics.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

u_int32_t my_send(const void *token, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
        MPI_Send(&token, 1, MPI_INT, dest+1, 0, MPI_COMM_WORLD);
        token = 0;
	printf("[%i] Token sent to process %i\n", dest, dest+1);
	return 0;
}

u_int32_t my_recv(const void *token, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status* status){
	MPI_Recv(&token, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sleep(1); //Printf() sync
        printf("[%i] Token received from process %i\n", source, source-1);

	return 0;
}

