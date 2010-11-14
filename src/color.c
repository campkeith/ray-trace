#include "color.h"

color color_scale (float s, color c)
{
    return (color){s * c.r, s * c.g, s * c.b};
}

color color_multiply (color a, color b)
{
    return (color){a.r * b.r, a.g * b.g, a.b * b.b};
}

color color_add (color a, color b)
{
    return (color){a.r + b.r, a.g + b.g, a.b + b.b};
}

bool is_color (color c)
{
    return c.r || c.g || c.b;
}
