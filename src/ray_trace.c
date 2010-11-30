#include "ray_trace.h"
#include "surface.h"
#include "vector.h"
#include "scene.h"

#include <math.h>
#include <string.h>
#include <stdbool.h>

/* This module implements the core recursive ray tracing algorithm used to render a scene.

   It implements the cast_ray function which takes a ray and scene parameters and determines the
   color of that ray.  The ray tracing framework calls cast_ray for each pixel in the image,
   producing a rendering of the scene.

   The color of a ray is determined by recursively tracing the tree of paths it takes through a
   given scene as it reflects and refracts about surfaces, up to a recursion depth limit.  Light
   contributions from each branch of the tree are added together to determine the color of the ray
   in question.  Ray cast beyond the depth limit are simply given the color of the background to
   avoid infinite recursion.

   The recursive algorithm for computing the color of a ray is as follows:

   Determine first surface hit, and determine intersection point and normal.
   If no surface is hit, the ray color is the background color
   Otherwise, depending on the properties of that surface, do some of the following:

   Use Fresnel's equations and Snell's law to determine reflection coefficient, transmission coefficient,
   and refracted ray.  Determine transmission subtotal as follows:
   Transmission subtotal = c_transmitted * specular part * refracted ray color

   Calculate reflected ray and determine reflection subtotal as follows:
   Reflection subtotal = c_reflected * specular part * reflected ray color

   Test shadow rays to determine which lights are illiminating the intersection point.  Use the cosine
   shading model to calculate diffuse coefficients for each.  Calculate the diffuse subtotal as follows:
   Diffuse subtotal = diffuse part * (c_diffuse_1 * light_color_1 + c_diffuse_2 * light_color_2 + ...)

   Add the calculated subtotals together to determine the color of the ray: 
   ray color = transmitted + reflected + diffuse
*/

bool get_intersection (vector origin, vector ray, surface * test_surface,
                       vector * intersection_out, vector * normal_out)
/*! Determine if the ray with the given origin and direction intersects with the given surface.
    If it does, set the insersection_out parameter (if not NULL) to indicate the location of the
    first intersection, set the normal_out parameter (if not NULL) to indicate the normal of the
    surface at the intersection point, and return true.
    If the ray never intersects the surface, return false.
*/
{
    return test_surface->class->calculate_intersection(origin, ray, test_surface->geometry,
                                                       intersection_out, normal_out);
}

float fresnel_refraction (vector ray, vector normal, float refraction_index,
                          vector * refracted_ray_out)
/*! Determine how the given ray will refract through the surface with the given normal and
    relative refraction index using Fresnel's equations.  Due to the wave nature of light,
    tracing the reflection/refraction backward is conveniently equivalent to tracing it forward.

    A reflection coefficient is returned indicating how much of traced ray's power will come
    from reflection.  This value will be between 0 (no light reflects) and 1 (all light reflects).
    The remainder of the ray's power is transmitted (refracted).  In other words:
    c_reflected + c_transmitted = 1

    If the ray does refract (i.e. the reflection coefficient is not 1) then the
    refracted_ray_out parameter will be set to indicate the direction of the refracted ray.
*/
{
    float cos_i, cos_t, cos2_t;
    float n, r_s, r_p;

    if (refraction_index == .0f)
    {
        /* No refraction index means reflection only */
        return 1.0f;
    }

    /* Determine cosine of incident ray wrt normal */
    cos_i = -dot_product(ray, normal);
    if (cos_i < 0)
    {
        /* Reverse the normal and refraction index if we're inside a surface */
        n = refraction_index;
        normal = vector_negate(normal);
        cos_i = -cos_i;
    }
    else
    {
        n = 1.0f / refraction_index;
    }

    /* Snell's law + pythagorean trigonometric identity to determine cosine of
       transmitted ray wrt normal on the other side of the surface */
    cos2_t = 1.0f - square(n) * (1.0f - square(cos_i));
    if (cos2_t < 0)
    {
        /* Imaginary square root: total internal reflection */
        return 1.0f;
    }
    cos_t = sqrtf(cos2_t);

    /* Determine refracted ray 
       http://www.cs.unc.edu/~rademach/xroads-RT/RTarticle.html#reflection */
    *refracted_ray_out = vector_add(vector_multiply(n, ray), vector_multiply(n * cos_i - cos_t, normal));

    /* Fresnel's equations for reflection coefficient
       http://en.wikipedia.org/wiki/Fresnel_equations */
    r_s = square((n * cos_i - cos_t) / (n * cos_i + cos_t));
    r_p = square((n * cos_t - cos_i) / (n * cos_t + cos_i));

    return 0.5f * (r_s + r_p);
}

vector reflect_ray (vector ray, vector surface_normal)
/*! Determine the reflection of a given ray from a surface with the given
    normal vector */
{
    float scale = -2.0f * dot_product(ray, surface_normal);
    return vector_add(ray, vector_multiply(scale, surface_normal));
}

float get_diffuse_coefficient (vector point, vector normal, light_source * source)
/*! Determine the diffuse coefficient of a given point on a surface with a given normal
    being lit by the given light source according to the cosine shading model. */
{
    float cos_theta;
    cos_theta = dot_product(normal, vector_normalize(vector_sub(source->position, point)));
    return fmax(cos_theta, .0f);
}

surface * hit_surface (vector origin, vector ray, surface surfaces[],
                       vector * intersection_out, vector * normal_out)
/*! Given a ray with given origin and direction and surface array as defined in scene.h,
    determine the closest surface hit.  The vector_distance function (defined in vector.h)
    will be useful here.  Set the output parameters intersection_out and normal_out to
    the intersection location of the closest surface and surface normal at that intersection,
    respectively.  Return a pointer to the closest surface hit.

    If the ray intersects none of the given surfaces, return NULL
*/
{
    surface * closest_surface = NULL;
    /* Implement me! */
    return closest_surface;
}

bool in_light (light_source * source, vector point, surface surfaces[])
/*! Determine if a given point in space is in the light of the given light source,
    meaning none of the given surfaces are blocking the line from the point to
    the light source.

    If the point is in the light, return true, otherwise return false.
*/
{
    vector shadow_ray;
    /* Ray directions must be normalized, since the ray tracing framework
       assumes they are.  The shadow ray is calculated and normalized for you below. */
    shadow_ray = vector_normalize(vector_sub(source->position, point));
    /* Implement me! */
    return true;
}

color get_illumination (vector point, vector ray, vector normal,
                        light_source light_sources[], surface surfaces[])
/*! Determine the diffuse illumination of the given point on a surface with the given
    normal with incident ray given for reference.  The illumination is calculated by
    summing the contributions of each light source with a direct line of sight to the
    point (no surfaces blocking), each contribution multiplied by the diffuse
    coefficient calculated by the get_diffuse_coefficient function.
*/
{
    color illumination = { .0f, .0f, .0f };
    /* The normal needs to be inverted if we're inside a surface before calculating
       illumination.  This is done for you below */
    if (dot_product(ray, normal) > 0)
    {
        normal = vector_negate(normal);
    }
    /* Implement me! */
    return illumination;
}

color cast_ray (scene * scene, vector origin, vector ray, int depth)
/*! Determine the color of a ray with given origin and direction by recursively tracing the path
    it takes through a given scene with recursion depth limit "depth".  Once the depth limit is
    reached (remaining depth is 0), rays are assumed to have the color of the scene background.

    The algorithm for computing the ray color is documented above.  Color multiplication and
    addition functions defined in color.h will be useful here.
*/
{
    color result = { .0f, .0f, .0f };
    /* Implement me!
       Be careful to avoid unnecessary function calls, especially ray casting calls for
       rays that will have no contribution! */
    return result;
}
