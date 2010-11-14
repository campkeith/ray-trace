#pragma once

#include <math.h>

typedef struct
{
    float x;
    float y;
    float z;
} vector;

vector vector_theta_phi (float theta, float phi);
vector vector_rotate (vector v, float theta, float phi);

void vector_print (vector v, char * label);

/* The rest of the vector functions are defined "inline" in the header
   file.  This tells the compiler to insert the complete body of the
   function wherever it's used.  This is a reasonable optimization
   since it has been shown to increase render speed ~110%
*/

static __inline float square (float x)
{
    return x * x;
}

static __inline vector vector_add (vector a, vector b)
{
    return (vector){a.x + b.x, a.y + b.y, a.z + b.z}; 
}

static __inline vector vector_sub (vector a, vector b)
{
    return (vector){a.x - b.x, a.y - b.y, a.z - b.z};
}

static __inline vector vector_multiply (float scalar, vector v)
{
    return (vector){scalar * v.x, scalar * v.y, scalar * v.z};
}

static __inline vector vector_negate (vector v)
{
    return (vector){-v.x, -v.y, -v.z};
}

static __inline float dot_product (vector a, vector b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static __inline vector cross_product (vector a, vector b)
{
    return (vector){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

static __inline float squared_magnitude (vector v)
{
    return square(v.x) + square(v.y) + square(v.z);
}

static __inline float vector_magnitude (vector v)
{
    return sqrtf(squared_magnitude(v));
}

static __inline vector vector_normalize (vector v)
{
    float magnitude = vector_magnitude(v);
    return (vector){v.x / magnitude, v.y / magnitude, v.z / magnitude};
}

static __inline vector vector_project (vector a, vector b)
{
    return vector_multiply(dot_product(a, b), b);
}

static __inline vector vector_orth (vector a, vector b)
{
    return vector_sub(a, vector_project(a, b));
}

static __inline float vector_distance (vector a, vector b)
{
    return sqrtf(square(a.x - b.x) + square(a.y - b.y) + square(a.z - b.z));
}
