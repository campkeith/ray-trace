#pragma once

#include "surface.h"
#include "vector.h"
#include "color.h"

typedef enum
{
    LIGHT_SOURCE_STANDARD,
    LIGHT_SOURCE_SENTINEL,
} light_source_type;

typedef struct
{
    light_source_type type;
    vector position;
    color color;
} light_source;

typedef struct
{
    surface_type type;
    intersection_function * calculate_intersection; 
    float refraction_index;
    color specular_part;
    color diffuse_part;
    void * user_data;
} surface;

typedef struct
{
    vector position;
    float theta;
    float phi;
    float view_angle;
    int pixels_wide;
    int pixels_high;
} aperture;

typedef struct
{
    color background_color;
    aperture aperture;
    light_source * light_sources;
    surface * surfaces;
} scene;
