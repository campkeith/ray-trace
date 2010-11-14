#pragma once

#include <stdbool.h>

#include "vector.h"

typedef bool intersection_function (vector origin, vector ray, void * data,
                                    vector * intersection_out, vector * normal_out);

typedef enum
{
    SURFACE_SPHERE,
    SURFACE_FRUSTUM,
    SURFACE_CIRCLE,
    SURFACE_QUAD,
    SURFACE_SENTINEL,
} surface_type;

/* A sphere is defined by a center and a radius */
typedef struct
{
    vector center;
    float radius;
} sphere;
intersection_function sphere_intersect;

/* A frustum is defined by a center line with two endpoints and two radii.
   This defines the curved surface of the frustum, not the circular caps */
typedef struct
{
    vector centers[2];
    float radii[2];
} frustum;
intersection_function frustum_intersect;

/* A circle is defined by a center, a normal to the circle plane, and a radius */
typedef struct
{
    vector center;
    vector normal;
    float radius;
} circle;
intersection_function circle_intersect;

/* A quad is defined by three consecutive vertices */
typedef struct
{
	vector vertices[3];
} quad;
intersection_function quad_intersect;
