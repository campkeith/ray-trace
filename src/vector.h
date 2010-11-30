#pragma once

#include <math.h>

/* This module defines a 3D vector data type and operations on those
   vectors.  Vectors are composed of three coordinates: x, y, z
   corresponding to the x-axis, y-axis, and z-axis parallel components.
   Vector can represent both points in 3D space and directions (such
   as the direction of a ray).  Vectors representing points can have
   any magnitude, while vectors representing directions are normalized
   to have magnitude 1 by convention.  Since many functions assume that
   vectors representing direction are normalized, it is best to
   normalize direction vectors as soon as they are created with the
   vector_normalize function.
*/

typedef struct
{
    float x;
    float y;
    float z;
} vector;

/*! Create a unit vector from spherical coordinates */
vector vector_theta_phi (float theta, float phi);

/*! Rotate the given vector using spherical coordinate offsets */
vector vector_rotate (vector v, float theta, float phi);

/*! Print the given vector for debugging purposes with the given string label */
void vector_print (vector v, char * label);

/* The rest of the vector functions are defined "inline" in the header
   file.  This tells the compiler to insert the complete body of the
   function wherever it's used.  This is a reasonable optimization
   since it has been shown to increase render speed ~110%
*/

/*! Compute the square of a floating point number */
static __inline float square (float x)
{
    return x * x;
}

/*! Compute the sum of two vectors, return the result */
static __inline vector vector_add (vector a, vector b)
{
    return (vector){a.x + b.x, a.y + b.y, a.z + b.z}; 
}

/*! Subtract vector b from vector a, return the result */
static __inline vector vector_sub (vector a, vector b)
{
    return (vector){a.x - b.x, a.y - b.y, a.z - b.z};
}

/*! Multiply vector v by the given scalar, return the result */
static __inline vector vector_multiply (float scalar, vector v)
{
    return (vector){scalar * v.x, scalar * v.y, scalar * v.z};
}

/*! Return the vector opposite vector v */
static __inline vector vector_negate (vector v)
{
    return (vector){-v.x, -v.y, -v.z};
}

/*! Compute the dot product of a and b, return the result */
static __inline float dot_product (vector a, vector b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*! Compute the cross product of a and b, return the result */
static __inline vector cross_product (vector a, vector b)
{
    return (vector){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

/*! Return the square of the magnitude of vector v */
static __inline float squared_magnitude (vector v)
{
    return square(v.x) + square(v.y) + square(v.z);
}

/*! Return the magnitude of vector v */
static __inline float vector_magnitude (vector v)
{
    return sqrtf(squared_magnitude(v));
}

/*! Normalize vector v, returning the unit vector with the same direction */
static __inline vector vector_normalize (vector v)
{
    float magnitude = vector_magnitude(v);
    return (vector){v.x / magnitude, v.y / magnitude, v.z / magnitude};
}

/*! Project vector a onto vector b, vector b being a unit vector,
    returning the component of vector a parallel to vector b */
static __inline vector vector_project (vector a, vector b)
{
    return vector_multiply(dot_product(a, b), b);
}

/*! Compute vector rejection of vector a onto vector b, vector b being a
    unit vector, returning the component of vector a orthagonal to vector b */
static __inline vector vector_orth (vector a, vector b)
{
    return vector_sub(a, vector_project(a, b));
}

/*! Return the magnitude of the difference between vector a and vector b */
static __inline float vector_distance (vector a, vector b)
{
    return sqrtf(square(a.x - b.x) + square(a.y - b.y) + square(a.z - b.z));
}
