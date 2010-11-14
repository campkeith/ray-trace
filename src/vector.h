#pragma once

typedef struct
{
    float x;
    float y;
    float z;
} vector;

float square (float x);

vector vector_add (vector a, vector b);
vector vector_sub (vector a, vector b);
vector vector_multiply (float scalar, vector v);

float dot_product (vector a, vector b);
vector cross_product (vector a, vector b);
vector vector_normalize (vector v);
vector vector_negate (vector v);

vector vector_project (vector a, vector b);
vector vector_orth (vector a, vector b);

float vector_magnitude (vector v);
float squared_magnitude (vector v);
float vector_distance (vector a, vector b);

vector vector_theta_phi (float theta, float phi);
vector vector_rotate (vector v, float theta, float phi);

void vector_print (vector v, char * label);
