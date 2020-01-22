#include <pthread.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lib/mechanics.h"
//#include "lib/detector.h"
//#include "lib/msg.h"
//#include "lib/token.h"
//#include "lib/retrans.h"

#define PROC_COUNT 4
#define SIZE 9

static volatile int received_message;
pthread_mutex_t lock;

void* timeout(void *source) {
    int volatile msec, trigger, source_node; /* 500ms */
    clock_t before;
    int *message;

    source_node = *((int*) source);
    while (1) {
        msec = 0;
        trigger = 10000;
        before = clock();
        do {
 
            clock_t difference = clock() - before;
            msec = difference * 1000 / CLOCKS_PER_SEC;
            pthread_mutex_lock(&lock);
            if(received_message > 0) {
                received_message = 0;
                before = clock();
            }
            pthread_mutex_unlock(&lock);
        } while ( msec < trigger );
        message = malloc(sizeof(int) * SIZE);
        memset(message, 0, SIZE * sizeof(int));
        message[0] = 1;
        message[3] = source_node;
        //printf("Sending detector to [%i]\n", source_node);
        send_message(message, SIZE, source_node + 1);
        free(message);
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    int my_token, rank, size, dest, has_token, num_of_crits, target_node;
    pthread_t timeout_thread;
    int *message; // [detector, token, retrans, source, retr_source, [processes...]]
    my_token = 0;
    // Initialize MPI environment
    MPI_Init (&argc, &argv);
    
    message = malloc(sizeof(int) * SIZE);
    memset(message, 0, SIZE * sizeof(int));

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
    
    if (size < 2) {
        printf("Too few processes\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    if (rank == 0) {
        my_token = 1;
        message[1] = 1;
        message[4] = rank;
        send_message(message, SIZE, rank + 1);
        num_of_crits += 1;
    }
    
    while (1) {
        memset(message, 0, 5 * sizeof(int));
        recv_message(message, SIZE, rank - 1);
         printf("R: [%i} : [%i], [%i], [%i], [%i], [%i], [%i], [%i], [%i], [%i]\n", 
		                rank, message[0], message[1], message[2], message[3], message[4], message[5], message[6], message[7], message[8]);
	
        switch (message[0]) {
            case 0:
                if (message[1]) {
                    sleep(1);
                    num_of_crits += 1;
                    message[1] +=1; //incrementing token
                    if (my_token < message[1]){ //check if retransmited token is not obsolete
                        my_token = message[1]; 
                        message[1] = my_token;
                        send_message(message, SIZE, rank + 1);
                        pthread_mutex_lock(&lock);
                        received_message = 1; 
                        pthread_mutex_unlock(&lock); 
                    }
                }
                else {
                    if (message[4] == rank) {
                        int *token;
                        token = malloc(sizeof(int) * SIZE);
                        memset(token, 0, SIZE * sizeof(int));
                        token[1] = my_token; 
                        send_message(token, SIZE, rank + 1);
                        free(token);
                    }
                    else 
                        send_message(message, SIZE, rank + 1);
                }

                break;
            
            case 1:
                if (message[3] == rank) {
                    target_node = find_max(message) - 5;
                    int *retrans;
                    retrans = malloc(sizeof(int) * SIZE);
                    memset(retrans, 0, SIZE * sizeof(int));
                    retrans[2] = 1;
                    retrans[4] = target_node;
                    send_message(retrans, SIZE, rank + 1);
                    free(retrans);
                }
                else {
                    message[rank + 5] = my_token;
                    send_message(message, SIZE, rank + 1);
                }

                break;

        }
    }
   
    MPI_Finalize();
    pthread_join(timeout_thread, NULL);

    return 0;
}
