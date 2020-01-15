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
	else
		printf("FAILED TO SEND FROM [%i]\n", dest);
	return 0;
}

u_int32_t recv_message(struct msg *message, int count, int source, int my_rank, u_int32_t *process_token_id){
	MPI_Recv(&message, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	switch(message->type) {
		case MPI_DETEC:
			message->detec->procList[my_rank] = *process_token_id;			 
			break;
		case MPI_TOKEN:
			*process_token_id = message->tok->count + 1;
			break;
	}

	sleep(1); //Printf() sync
    printf("[%i] Message received from process %i\n", source, source-1);

	return 0;
}

