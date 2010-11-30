#pragma once

#include <stdbool.h>

/* This module defines an RGB color and operations on those colors.
   Colors are composed of red (r) green (g) and blue (b) components,
   each of which is a floating point value between 0 and 1.  Colors
   are used to represent the color of a light ray and of a surface.
   For a light source or light ray, 0 is minimum intensity and 1
   is maximum intensity. (values greater than 1 are okay; they
   represent overexposure)  For diffuse or specular components of a
   surface, 0 represents no reflection or transmission of that color
   component while 1 represents reflection or transmission of all of
   that color component.
*/

typedef struct
{
    float r;
    float g;
    float b;
} color;

/*! Multiply all components of a color by a given constant.
    Useful for multiplying by a scalar coefficient. */
color color_scale (float s, color c);

/*! Multiply corresponding components of two colors together
    Useful for multiplying surface colors with light colors */
color color_multiply (color a, color b);

/*! Add corresponding components of two colors together */
color color_add (color a, color b);

/*! Determine if any color component is nonzero.
    Useful for determining if a surface "has" a color component */
bool is_color (color c);
