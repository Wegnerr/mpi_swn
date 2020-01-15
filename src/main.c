#include <pthread.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include "lib/mechanics.h"
#include "lib/mechanics.c"
#include "lib/detector.h"
#include "lib/msg.h"
#include "lib/msg.h"
#include <unistd.h>

int receivedMessage;
pthread_mutex_t lock;

void* timeout(void *source) {
    int msec, trigger; /* 500ms */
    clock_t before;
    struct msg* message;

    msec = 0;
    trigger = 500;
    before = clock();
    do {
 
        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
        
        pthread_mutex_lock(&lock);
        if(receivedMessage > 0)
            msec = 0;
            receivedMessage = 0;
        pthread_mutex_unlock(&lock);
    
    } while ( msec < trigger );
   
    message->tok = NULL;
    message->detec = malloc(sizeof(struct detector));
    send_message(message, 1, MPI_INT, *((int*)source)+1, 0, MPI_COMM_WORLD);
    return NULL;
}


int main(int argc, char *argv[]) {
    int rank, size, dest, hasToken;
    struct msg message;
    pthread_t timeout_thread;

    // Initialize MPI environment
    MPI_Init (&argc, &argv);

    // Get rank, size and processor name
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if(pthread_create(&timeout_thread, NULL, timeout, &rank)) {
        fprintf(stderr, "Error creating thread\n");
    return 1;

    }

    pthread_mutex_lock(&lock);
    receivedMessage = 0; 
    pthread_mutex_unlock(&lock); 


    //TO-DO
    if (size < 2) {
        fprintf(stderr, "Number of processes must be larger than 1 in order to run this example\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }if (rank == 0) {
        hasToken = 1;
        send_message(&message, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        recv_message(&message, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        recv_message(&message, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	if (rank == size - 1)
            dest = 0;
        else
            dest = rank + 1;
	send_message(&message, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    }

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
    }
    // Finalize MPI environment
    MPI_Finalize();
    pthread_join(timeout_thread, NULL);

    return 0;
}
