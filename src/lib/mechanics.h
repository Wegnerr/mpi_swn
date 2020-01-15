#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include <mpi.h>

u_int32_t send_message(struct msg *message, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

u_int32_t recv_message(struct msg *message, int count, int source, int my_rank, u_int32_t *process_token_id);