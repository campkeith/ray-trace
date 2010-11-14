#pragma once

#include <stdbool.h>

typedef struct
{
    float r;
    float g;
    float b;
} color;

color color_scale (float s, color c);
color color_multiply (color a, color b);
color color_add (color a, color b);
bool is_color (color c);
