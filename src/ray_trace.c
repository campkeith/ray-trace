#include "ray_trace.h"
#include "surface.h"
#include "vector.h"
#include "scene.h"

#include <math.h>
#include <string.h>
#include <stdbool.h>

static bool calculate_intersection (vector origin, vector ray, surface * cur_surface,
                                    vector * intersection_out, vector * normal_out)
{
	return cur_surface->class->calculate_intersection(origin, ray, cur_surface->geometry,
                                                      intersection_out, normal_out);
}

static vector reflect_ray (vector ray, vector surface_normal)
{
    float scale = -2.0f * dot_product(ray, surface_normal);
    return vector_add(ray, vector_multiply(scale, surface_normal));
}

static float refract_ray (vector ray, vector normal, float refraction_index, vector * ray_out)
{
    ray = vector_normalize(ray);
    normal = vector_normalize(normal);
    float cos_i, cos_t;
    vector refracted_ray;
    float n, r_s, r_p;
    
    if (refraction_index == .0f)
    {
        return 1.0f;
    }
    
    cos_i = -dot_product(ray, normal);
    if (cos_i < 0)
    {
        n = refraction_index;
        normal = vector_negate(normal);
        cos_i = -cos_i;
    }
    else
    {
        n = 1.0f / refraction_index;
    }
    float determinant = 1.0f - square(n) * (1.0f - square(cos_i));
    if (determinant < 0)
    {
        return 1.0f;
    }
    float c2 = sqrtf(determinant);
    refracted_ray = vector_add(vector_multiply(n, ray), vector_multiply(n * cos_i - c2, normal));
    *ray_out = refracted_ray;

    cos_t = -dot_product(normal, refracted_ray);


    r_s = square((n * cos_i - cos_t) / (n * cos_i + cos_t));
    r_p = square((n * cos_t - cos_i) / (n * cos_t + cos_i));
    
    return 0.5f * (r_s + r_p);
}

static surface * hit_surface (vector origin, vector ray, surface surfaces[],
                              vector * intersection_out, vector * normal_out)
{
    float closest_distance = INFINITY;
    surface * closest_surface = NULL;
    surface * cur_surface;
    for (cur_surface = surfaces; cur_surface->class != NULL; cur_surface++)
    {
        vector intersection, normal;
        if (calculate_intersection(origin, ray, cur_surface, &intersection, &normal))
        {
            float distance = vector_distance(intersection, origin);
            if (distance < closest_distance)
            {
                closest_distance = distance;
                closest_surface = cur_surface;
                *intersection_out = intersection;
                *normal_out = normal;
            }
        }
    }
    return closest_surface;
}

static bool in_light (light_source * source, vector point, surface surfaces[])
{
    float source_distance;
    vector ray;
    surface * cur_surface;
    source_distance = vector_distance(source->position, point);
    ray = vector_normalize(vector_sub(source->position, point));
    for (cur_surface = surfaces; cur_surface->class != NULL; cur_surface++)
    {
        vector intersection;
        if (calculate_intersection(point, ray, cur_surface, &intersection, NULL))
        {
            float distance = vector_distance(intersection, point);
            if (1e-3 < distance && distance < source_distance)
            {
                return false;
            }
        }
    }
    return true;
}

static color calc_illumination (vector point, vector ray, vector normal,
                                light_source light_sources[], surface surfaces[])
{
    if (dot_product(ray, normal) > .0f)
    {
        normal = vector_negate(normal);
    }
    color illumination = { .0f, .0f, .0f };
    float c;
    light_source * source;
    for (source = light_sources; source->type != LIGHT_SOURCE_SENTINEL; source++)
    {
        if (in_light(source, point, surfaces))
        {
            c = dot_product(normal, vector_normalize(vector_sub(source->position, point)));
            if (c > .0f)
            {
                illumination = color_add(illumination, color_scale(c, source->color));
            }
        }
    }
    return illumination;
}

color cast_ray (scene * scene, vector origin, vector ray, int depth)
{
    vector refracted_ray, reflected_ray, intersection, normal = {};
    color result = { .0f, .0f, .0f };
    color refracted_color, reflected_color;
    surface * surf;
    float refl_coef;
    
    if (depth == 0)
    {
        return scene->background_color;
    }
    
    surf = hit_surface(origin, ray, scene->surfaces, &intersection, &normal);
    if (surf == NULL)
    {
        return scene->background_color;
    }

    if (is_color(surf->specular_part))
    {
        refl_coef = refract_ray(ray, normal, surf->refraction_index, &refracted_ray);
        if (refl_coef != 1.0f)
        {
            refracted_color = cast_ray(scene, intersection, refracted_ray, depth - 1);
            result = color_add(result, color_scale(1 - refl_coef, color_multiply(surf->specular_part, refracted_color)));
        }

        reflected_ray = reflect_ray(ray, normal);
        reflected_color = cast_ray(scene, intersection, reflected_ray, depth - 1);
        result = color_add(result, color_scale(refl_coef, color_multiply(surf->specular_part, reflected_color)));
    }

    if (is_color(surf->diffuse_part))
    {
        color diffuse = calc_illumination(intersection, ray, normal,
                                          scene->light_sources, scene->surfaces);
        result = color_add(result, color_multiply(surf->diffuse_part, diffuse));
    }

    return result;
}
