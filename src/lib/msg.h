#pragma once
#include "token.h"
#include "detector.h"
#include <stdint.h>

struct msg {
    struct token* tok;
    struct detector* detec;
};