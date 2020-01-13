#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

u_int32_t my_send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

u_int32_t my_recv(const void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status* status);

