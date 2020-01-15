#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include <mpi.h>

u_int32_t send_message(struct msg *message, int count, int dest, int tag);

u_int32_t recv_message(struct msg *message, int count, int source);

u_int32_t find_max(int node_list[]);