#include <pthread.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include "lib/mechanics.h"
#include "lib/mechanics.c"
#include "lib/detector.h"
#include "lib/msg.h"
#include "lib/token.h"
#include <unistd.h>

int receivedMessage;
pthread_mutex_t lock;

void* timeout() {
    int msec = 0, trigger = 500; /* 500ms */
    clock_t before = clock();

    do {
 
        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
        
        pthread_mutex_lock(&lock);
        if(receivedMessage > 0)
            msec = 0;
            receivedMessage = 0;
        pthread_mutex_unlock(&lock);
    
    } while ( msec < trigger );
    //my_send();
    return NULL;
}


/*
Compile with mpicc main.c -o main
Run 	with mpirun -np <number_of_nodes> main

*/
int main(int argc, char *argv[]) {
    int rank, size, token, dest;
    struct msg message;
    struct token tok;
    struct detector detec;
    pthread_t timeout_thread;
    if(pthread_create(&timeout_thread, NULL, timeout, NULL)) {
        fprintf(stderr, "Error creating thread\n");
    return 1;

    }

    pthread_mutex_lock(&lock);
    receivedMessage = 0; 
    pthread_mutex_unlock(&lock); 

    // Initialize MPI environment
    MPI_Init (&argc, &argv);

    // Get rank, size and processor name
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (size < 2) {
        fprintf(stderr, "Number of processes must be larger than 1 in order to run this example\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }if (rank == 0) {
        token = 1;
        my_send(&token, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        my_recv(&token, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        my_recv(&token, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	if (rank == size - 1)
            dest = 0;
        else
            dest = rank + 1;
	my_send(&token, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
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
