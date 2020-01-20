#include "mechanics.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>

#define SIZE 9

u_int32_t send_message(int *message, int count, int dest){
    
	srand(time(NULL));

	if (dest > PROC_COUNT - 1)
		dest = 0;
	
	//if (rand() % 100 > 10)
	{
		MPI_Send(message, count, MPI_INT, dest, 0, MPI_COMM_WORLD);
		printf("[%i] Message sent to process %i\n", dest - 1, dest);
	}
	//else
	//	printf("FAILED TO SEND FROM [%i]\n", dest);
	return 0;
}

u_int32_t recv_message(int *message , int count, int source){
	if (source < 0)
		source = PROC_COUNT - 1;

	MPI_Recv(message, count, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("[%i] Message received from process %i\n", source, source-1);

	return 0;
}

u_int32_t find_max(int node_list[]) {
	size_t size;
    int i, max, index;
    
	index = 0;
    max = node_list[0];
    for (i = 5 ; i < SIZE; i++) {
        if (node_list[i] > max) {
            max = node_list[i];
			index = i;
		}
    } 

    return index;
}