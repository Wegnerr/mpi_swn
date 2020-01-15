#include "mechanics.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>

u_int32_t send_message(struct msg *message, int count, int dest, int tag){
    
	srand(time(NULL));

	if (dest > PROC_COUNT - 1)
		dest = 0;
	
	if (rand() % 100 > 10)
	{
		MPI_Send(message, 1, MPI_INT, dest+1, 0, MPI_COMM_WORLD);
		printf("[%i] Message sent to process %i\n", dest - 1, dest);
	}
	else
		printf("FAILED TO SEND FROM [%i]\n", dest);
	return 0;
}

u_int32_t recv_message(struct msg *message, int count, int source){
	if (source < 0)
		source = PROC_COUNT - 1;

	MPI_Recv(message, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[%i] Message received from process %i\n", source, source-1);

	return 0;
}

u_int32_t find_max(int node_list[]) {
	size_t size;
    int i, max;
    
    max = node_list[0];
    for (i = 0; i < PROC_COUNT; i++) {
        if (node_list[i] > max)
            max = node_list[i];
    } 

    return max;
}