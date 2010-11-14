#include "vector.h"

#include <stdio.h>
#include <math.h>

float square (float x)
{
    return x * x;
}

vector vector_add (vector a, vector b)
{
    return (vector){a.x + b.x, a.y + b.y, a.z + b.z}; 
}

vector vector_sub (vector a, vector b)
{
    return (vector){a.x - b.x, a.y - b.y, a.z - b.z};
}

vector vector_multiply (float scalar, vector v)
{
    return (vector){scalar * v.x, scalar * v.y, scalar * v.z};
}

vector vector_negate (vector v)
{
    return (vector){-v.x, -v.y, -v.z};
}

float dot_product (vector a, vector b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vector cross_product (vector a, vector b)
{
    return (vector){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

float vector_magnitude (vector v)
{
    return sqrtf(squared_magnitude(v));
}

float squared_magnitude (vector v)
{
    return square(v.x) + square(v.y) + square(v.z);
}

vector vector_normalize (vector v)
{
    float magnitude = vector_magnitude(v);
    return (vector){v.x / magnitude, v.y / magnitude, v.z / magnitude};
}

vector vector_project (vector a, vector b)
{
    return vector_multiply(dot_product(a, b), b);
}

vector vector_orth (vector a, vector b)
{
    return vector_sub(a, vector_project(a, b));
}

float vector_distance (vector a, vector b)
{
    return sqrtf(square(a.x - b.x) + square(a.y - b.y) + square(a.z - b.z));
}

vector vector_theta_phi (float theta, float phi)
{
    return (vector){ cosf(phi) * cosf(theta),
                     cosf(phi) * sinf(theta),
                     sinf(phi) };
}

vector vector_rotate (vector v, float theta, float phi)
{
    vector v1, v2;

    v1.x = v.x * cosf(phi) + v.z * -sinf(phi);
    v1.y = v.y;
    v1.z = v.x * sinf(phi) + v.z *  cosf(phi);

    v2.x = v1.x * cosf(theta) + v1.y * -sinf(theta);
    v2.y = v1.x * sinf(theta) + v1.y *  cosf(theta);
    v2.z = v1.z;
    
    return v2;
}

void vector_print (vector v, char * label)
{
    fprintf(stderr, "%s: (%.2f, %.2f, %.2f)\n", label, v.x, v.y, v.z);
}
