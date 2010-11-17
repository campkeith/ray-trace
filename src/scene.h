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

/* Defines image resolution, in pixels */
typedef struct
{
    int width;
    int height;
} resolution;

/* Defines a direction in the spherical coordinate system */
typedef struct
{
    float theta;
    float phi;
} direction;

typedef struct
{
    vector position;
    direction direction; /* Which way the camera is facing */
    resolution resolution;
    float view_angle; /* Horizontal camera view angle */
} camera;

typedef struct
{
    color background_color;
    camera camera;
    /* Pointer to an array of light sources.  The array is terminated
       with a light source of type LIGHT_SOURCE_SENTINEL */
    light_source * light_sources;
    /* Pointer to an array of surfaces.  The array is terminated with
       a surface with class NULL.  See surface.h for the definition of
       a surface */
    surface * surfaces;
} scene;
