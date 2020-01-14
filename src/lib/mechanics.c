#include "mechanics.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

u_int32_t send_message(struct msg *message, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    
	srand(time(NULL));

	if(rand() % 100 > 10)
	{
		MPI_Send(&message, 1, MPI_INT, dest+1, 0, MPI_COMM_WORLD);
		printf("[%i] Message sent to process %i\n", dest, dest+1);
	}
	return 0;
}

u_int32_t recv_message(struct msg *message, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status* status){
	MPI_Recv(&message, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sleep(1); //Printf() sync
    printf("[%i] Message received from process %i\n", source, source-1);

	return 0;
}

