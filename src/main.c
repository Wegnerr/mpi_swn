#include <pthread.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
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
    send_message(message, 1, source_node + 1, 0);
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
        send_message(message, 1, rank + 1, 0);
        has_token = 0;
    }
    
    while (1) {
        recv_message(message, 1, rank - 1);
        
        switch (message->type) {
            case MPI_DETEC:
                if (message->detec->proc_id == rank) {
                    target_node = find_max(message->detec->proc_list);
                    message->detec = NULL;
                    message->tok = NULL;
                    message->retr = malloc(sizeof(struct retrans));
                    message->type = MPI_RETRANS;
                    send_message(message, 1, rank + 1, 0);
                }
                else {
                    message->detec->proc_list[rank] = num_of_crits;
                    send_message(message, 1, rank + 1, 0);
                }
                break;
            
            case MPI_TOKEN:
                num_of_crits += 1;
                sleep(2);
                send_message(message, 1, rank + 1, 0);
                break;

            case MPI_RETRANS:
                if(message->retr->target_node == rank) {
                    message->detec = NULL;
                    message->retr = NULL;
                    message->tok = malloc(sizeof(struct token));
                    message->type = MPI_TOKEN;
                    send_message(message, 1, rank + 1, 0);
                }
                break;

        }
    }
    /*
    if (rank == 0) {
        //message = 1;
        MPI_Send(&message, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        //message = 0;
        printf("[%i] Token sent to process %i\n",
                rank, rank+1);
        MPI_Recv(&message, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sleep(1); //Print sync
        printf("[%i] Token received from process %i\n",
                rank, size-1);
    } else {
        MPI_Recv(&message, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sleep(1); //Print sync
        printf("[%i] Token received from process %i\n",
                rank, rank-1);
        if (rank == size - 1)
            dest = 0;
        else
            dest = rank + 1;
        MPI_Send(&message, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        //message = 0;
        printf("[%i] Token sent to process %i\n",
                rank, dest);
    } */
    // Finalize MPI environment
    MPI_Finalize();
    pthread_join(timeout_thread, NULL);

    return 0;
}
