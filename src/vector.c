#include "vector.h"

#include <stdio.h>
#include <math.h>

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
