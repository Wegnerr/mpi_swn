#pragma once
#include <stdint.h>

#define PROC_COUNT 4

struct detector {
    uint32_t proc_id;
    uint32_t proc_list[PROC_COUNT];
};
