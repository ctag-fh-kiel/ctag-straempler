#pragma once
#include <stdint.h>
#include "tft.h"


typedef struct{
    uint32_t x, y, w, h;
}bbox_t;

typedef struct{
    int16_t x;
    int16_t y;
}vec2d_t;
