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
	
	if (rand() % 100 > 10)
	{
		printf("S: [%i} : [%i], [%i], [%i], [%i], [%i], [%i], [%i], [%i], [%i]\n", 
		                dest, message[0], message[1], message[2], message[3], message[4], message[5], message[6], message[7], message[8]);
	
		MPI_Send(message, count, MPI_INT, dest, 0, MPI_COMM_WORLD);
	}
	else
		printf("FAILED TO SEND TO [%i]\n", dest);
	return 0;
}

u_int32_t recv_message(int *message , int count, int source){
	if (source < 0)
		source = PROC_COUNT - 1;

	MPI_Recv(message, count, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	return 0;
}

u_int32_t find_max(int node_list[]) {
	size_t size;
    int i, max, index;
    
	index = 5;
    max = node_list[0];
    for (i = 5 ; i < SIZE; i++) {
        if (node_list[i] > max) {
            max = node_list[i];
			index = i;
		}
    } 

    return index;
}
// [detector, token, retrans, source, retr_source, [processes...]]