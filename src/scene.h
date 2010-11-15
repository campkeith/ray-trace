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
	int width;
    int height;
} resolution;

typedef struct
{
	float theta;
    float phi;
} direction;

typedef struct
{
    vector position;
    direction direction;
    resolution resolution;
    float view_angle;
} aperture;

typedef struct
{
    color background_color;
    aperture aperture;
    light_source * light_sources;
    surface * surfaces;
} scene;
