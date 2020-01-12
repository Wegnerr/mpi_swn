#pragma once
#include <stdint.h>
#define PROC_COUNT 10
struct detector {
    uint32_t procId;
    uint32_t procList[PROC_COUNT];
};
