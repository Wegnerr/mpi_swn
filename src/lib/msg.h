#pragma once
#include "token.h"
#include "detector.h"
#include <stdint.h>

enum msg_type {
    MPI_DETEC,
    MPI_TOKEN,
};

struct msg {
    struct token* tok;
    struct detector* detec;
    enum msg_type type;
};

//TODO dodac msg zlecenia retransmisji