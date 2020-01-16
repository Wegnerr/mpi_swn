#include <pthread.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include "lib/mechanics.h"
#include "lib/detector.h"
#include "lib/msg.h"
#include "lib/token.h"
#include "lib/retrans.h"

int received_message;
pthread_mutex_t lock;

void* timeout(void *source) {
    int msec, trigger, source_node; /* 500ms */
    clock_t before;
    struct msg* message;

    source_node = *((int*) source);
    msec = 0;
    trigger = 5000;
    before = clock();
    do {
 
        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
        
        pthread_mutex_lock(&lock);
        if(received_message > 0)
            msec = 0;
            received_message = 0;
        pthread_mutex_unlock(&lock);
    
    } while ( msec < trigger );
   
    message->tok = NULL;
    message->detec = malloc(sizeof(struct detector));
    message->type = MPI_DETEC;
    memset(message->detec->proc_list, 0, PROC_COUNT);
    message->detec->proc_id = source_node;

    printf("Sending detector from [%i]\n", source_node);
   // send_message(message, 1, MPI_MSG_TYPE, source_node + 1, 0);
    return NULL;
}


int main(int argc, char *argv[]) {
    int rank, size, dest, has_token, num_of_crits, target_node;
    struct msg *message;
    pthread_t timeout_thread;

    message = malloc(sizeof(struct msg));
    // Initialize MPI environment
    MPI_Init (&argc, &argv);

    // Get rank, size and processor name
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    /* Creating MPI Structures */
    	//token
        int count_token = 1;
        int array_of_blocklengths_token[] = {1};
        MPI_Aint array_of_displacements_token[] = { offsetof(struct token, count) };
        MPI_Datatype array_of_types_token[] = {MPI_INT};
        MPI_Datatype tmp_type, MPI_TOKEN_TYPE;
        MPI_Aint lb, extent;
        MPI_Type_create_struct( count_token, array_of_blocklengths_token, array_of_displacements_token, array_of_types_token, &tmp_type);
        MPI_Type_get_extent( tmp_type, &lb, &extent);
        MPI_Type_create_resized( tmp_type, lb, extent, &MPI_TOKEN_TYPE);
        MPI_Type_commit( &MPI_TOKEN_TYPE);
	//retrans
	int count_retrans = 1;
	int array_of_blocklengths_retrans[] = {1};
	MPI_Aint array_of_displacements_retrans[] = { offsetof(struct retrans, target_node) };
	MPI_Datatype array_of_types_retrans[] = {MPI_INT};
	MPI_Datatype MPI_RETRANS_TYPE;

	MPI_Type_create_struct( count_retrans, array_of_blocklengths_retrans, array_of_displacements_retrans, array_of_types_retrans, &MPI_RETRANS_TYPE);
	
	//detector
	int count_detector = 2;
	int array_of_blocklengths_detector[] = {1, 1};
	MPI_Aint array_of_displacements_detector[] = { offsetof(struct detector, proc_id),
						       offsetof(struct detector, proc_list)};
	MPI_Datatype array_of_types_detector[] = {MPI_INT, MPI_INT};
	MPI_Datatype MPI_DETECTOR_TYPE;

	MPI_Type_create_struct( count_detector, array_of_blocklengths_detector, array_of_displacements_detector, array_of_types_detector, &MPI_DETECTOR_TYPE);

	//msg
	int count_msg = 4;
	int array_of_blocklengths_msg[] = {1, 1, 1, 1};
	MPI_Aint array_of_displacements_msg[] = { offsetof(struct msg, tok),
						  offsetof(struct msg, detec),
					  	  offsetof(struct msg, retr),
						  offsetof(struct msg, type) };
	MPI_Datatype array_of_types_msg[] = {MPI_TOKEN_TYPE, MPI_DETECTOR_TYPE, MPI_RETRANS_TYPE, MPI_INT};
	MPI_Datatype MPI_MSG_TYPE;
	MPI_Type_create_struct( count_msg, array_of_blocklengths_msg, array_of_displacements_msg, array_of_types_msg, &MPI_MSG_TYPE);
    /* END of MPI Strcutures */


    if (pthread_create(&timeout_thread, NULL, timeout, &rank)) {
        fprintf(stderr, "Error creating thread\n");
    return 1;
    }

    printf("[%i] Created\n", rank);

    num_of_crits = 0;

    pthread_mutex_lock(&lock);
    received_message = 0; 
    pthread_mutex_unlock(&lock); 

    message->detec = NULL;
    message->retr = NULL;
    message->tok = malloc(sizeof(struct token));
    message->type = MPI_TOKEN;

    //TO-DO
    if (size < 2) {
        fprintf(stderr, "Number of processes must be larger than 2 in order to run this example\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
 
    if (rank == 0) {
        has_token = 1;
        send_message(message, 1, MPI_MSG_TYPE, rank + 1, 0);
        has_token = 0;
    }
    
    while (1) {
	printf("W petli while");
        recv_message(message, 1, MPI_MSG_TYPE, rank - 1);
        
        switch (message->type) {
            case MPI_DETEC:
                if (message->detec->proc_id == rank) {
                    target_node = find_max(message->detec->proc_list);
                    message->detec = NULL;
                    message->tok = NULL;
                    message->retr = malloc(sizeof(struct retrans));
                    message->type = MPI_RETRANS;
                    send_message(message, 1, MPI_MSG_TYPE, rank + 1, 0);
                }
                else {
                    message->detec->proc_list[rank] = num_of_crits;
                    send_message(message, 1, MPI_MSG_TYPE, rank + 1, 0);
                }
                break;
            
            case MPI_TOKEN:
                num_of_crits += 1;
                sleep(2);
                send_message(message, 1, MPI_MSG_TYPE, rank + 1, 0);
                break;

            case MPI_RETRANS:
                if(message->retr->target_node == rank) {
                    message->detec = NULL;
                    message->retr = NULL;
                    message->tok = malloc(sizeof(struct token));
                    message->type = MPI_TOKEN;
                    send_message(message, 1, MPI_MSG_TYPE, rank + 1, 0);
                }
             break;

        }
    }
    
    // Finalize MPI environment
    MPI_Finalize();
    pthread_join(timeout_thread, NULL);

    return 0;
}
