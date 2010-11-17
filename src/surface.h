#pragma once

#include <stdbool.h>

#include "color.h"
#include "vector.h"

/* Object oriented programming in ANSI C!
   This framework effectively turns the surface struct into a class,
   with sphere, frustum, circle, and quad "flavors" inheriting from it.  Large
   projects (such as the Linux kernel) use conventions like this to do object
   oriented programming in C.

   How this works:
   Each surface maintains a "class" member which points to a structure which
   has all the specific functions for that class.  Each surface is also
   allocated some extra bytes of space to define the specific geometry
   of that surface.  This geometry data is passed to each specialized
   intersection function, which knows how to interpret it.

   How to initialize a surface:
   Set the class pointer to point to the appropriate surface class, one of
   surface_sphere, surface_frustum, surface_circle, surface_quad.
   Interpret the extra bytes as the appropriate structure, and fill it in.
*/

typedef bool intersection_function (vector origin, vector ray, void * geometry,
                                    vector * intersection_out, vector * normal_out);

typedef struct
{
    intersection_function * calculate_intersection;
} surface_class;

surface_class * surface_sphere;
surface_class * surface_frustum;
surface_class * surface_circle;
surface_class * surface_quad;

typedef struct
{
    surface_class * class;
    float refraction_index;
    color specular_part;
    color diffuse_part;
    /* Information about the geometry of the object
       is stored in the following 32 bytes.  */
    char geometry[32];
} surface;
/* The geometry bytes are to be interpreted according to the struct
   that corresponds to the surface's class.  Those struct definitions follow: */

/* A sphere is defined by a center and a radius */
typedef struct
{
    vector center;
    float radius;
} sphere;

/* A frustum is defined by a center line with two endpoints and two radii.
   This defines the curved surface of the frustum, not the circular caps */
typedef struct
{
    vector centers[2];
    float radii[2];
} frustum;

/* A circle is defined by a center, a normal to the circle plane, and a radius */
typedef struct
{
    vector center;
    vector normal;
    float radius;
} circle;

/* A quad is parallelogram defined by three consecutive vertices */
typedef struct
{
    vector vertices[3];
} quad;
