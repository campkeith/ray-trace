#include "surface.h"

#include <math.h>

const float f_min = 1e-2;

static intersection_function sphere_intersect, frustum_intersect,
                             circle_intersect, quad_intersect;

static surface_class surface_classes[] =
{
    { sphere_intersect },
    { frustum_intersect },
    { circle_intersect },
    { quad_intersect },
};

surface_class * surface_sphere = &surface_classes[0];
surface_class * surface_frustum = &surface_classes[1];
surface_class * surface_circle = &surface_classes[2];
surface_class * surface_quad = &surface_classes[3];

static bool solve_linear (vector origin, vector ray, vector plane_point, vector plane_normal,
                          vector * intersection_out)
{
    float t;
    t = dot_product(vector_sub(plane_point, origin), plane_normal) / dot_product(ray, plane_normal);
    if (t < f_min)
    {
        return false;
    }
    else
    {
        *intersection_out = vector_add(origin, vector_multiply(t, ray));
        return true;
    }
}

static int solve_quadratic (vector origin, vector ray, float a, float k, float c,
                            vector intersections_out[2])
{
    ray = vector_normalize(ray);
    float root;
    float base, delta;
    float determinant = square(k) - a * c;

    if (determinant < .0f)
    {
        return 0;
    }

    root = sqrtf(determinant);

    base = -k / a;
    delta = fabsf(root / a);

    if (base - delta > f_min)
    {
        intersections_out[0] = vector_add(origin, vector_multiply(base - delta, ray));
        intersections_out[1] = vector_add(origin, vector_multiply(base + delta, ray));
        return 2;
    }
    else if (base + delta > f_min)
    {
        intersections_out[0] = vector_add(origin, vector_multiply(base + delta, ray));
        return 1;
    }
    else
    {
        return 0;
    }
}

bool sphere_intersect (vector origin, vector ray, void * geometry,
                       vector * intersection_out, vector * normal_out)
{
    sphere * self = (sphere *)geometry;
    float k, c;
    vector relative_origin;
    vector intersections[2];

    relative_origin = vector_sub(origin, self->center);
    k = dot_product(ray, relative_origin);
    c = squared_magnitude(relative_origin) - square(self->radius);

    if (solve_quadratic(origin, ray, 1.0f, k, c, intersections) > 0)
    {
        if (normal_out)
        {
            *normal_out = vector_normalize(vector_sub(intersections[0], self->center));
        }
        if (intersection_out)
        {
            *intersection_out = intersections[0];
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool frustum_intersect (vector origin, vector ray, void * geometry,
                        vector * intersection_out, vector * normal_out)
{
    frustum * self = (frustum *)geometry;
    vector axis, relative_origin, relative_center;
    vector ray_orth, origin_orth;
    vector intersections[2];
    float a,k,c;
    float coefficient;
    float radius_linear, radius_constant;
    int index, num_hits;

    relative_origin = vector_sub(origin, self->centers[0]);
    relative_center = vector_sub(self->centers[1], self->centers[0]);
    axis = vector_normalize(relative_center);
    coefficient = (self->radii[1] - self->radii[0]) / vector_magnitude(relative_center);

    origin_orth = vector_orth(relative_origin, axis);
    ray_orth = vector_orth(ray, axis);
    radius_linear = coefficient * dot_product(ray, axis);
    radius_constant = coefficient * dot_product(relative_origin, axis) + self->radii[0];

    a = squared_magnitude(ray_orth) - square(radius_linear);
    k = dot_product(ray_orth, origin_orth) - radius_linear * radius_constant;
    c = squared_magnitude(origin_orth) - square(radius_constant);

    num_hits = solve_quadratic(origin, ray, a, k, c, intersections);
    for (index = 0; index < num_hits; index++)
    {
        if (dot_product(vector_sub(intersections[index], self->centers[0]), axis) >= .0f &&
            dot_product(vector_sub(intersections[index], self->centers[1]), axis) <= .0f)
        {
            if (intersection_out)
            {
                *intersection_out = intersections[index];
            }
            if (normal_out)
            {
                vector relative_intersection, axis_normal, cap_point, surface_tangent;
                float normal_origin;

                relative_intersection = vector_sub(intersections[index], self->centers[0]);
                axis_normal = vector_normalize(vector_orth(relative_intersection, axis));
                cap_point = vector_add(self->centers[0], vector_multiply(self->radii[0], axis_normal));
                surface_tangent = vector_normalize(vector_sub(intersections[index], cap_point));
                normal_origin = dot_product(relative_intersection, surface_tangent) / dot_product(surface_tangent, axis);
                *normal_out = vector_normalize(vector_sub(relative_intersection, vector_multiply(normal_origin, axis)));
            }
            return true;
        }
    }
    return false;
}

bool circle_intersect (vector origin, vector ray, void * geometry,
                       vector * intersection_out, vector * normal_out)
{
    circle * self = (circle *)geometry;
    vector intersection;

    if (!solve_linear(origin, ray, self->center, self->normal, &intersection))
    {
        return false;
    }
    if (vector_distance(intersection, self->center) <= self->radius)
    {
        if (normal_out)
        {
            *normal_out = self->normal;
        }
        if (intersection_out)
        {
            *intersection_out = intersection;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool quad_intersect (vector origin, vector ray, void * geometry,
                     vector * intersection_out, vector * normal_out)
{
    float proj1, proj2;
    vector intersection, relative_intersection;
    quad * self = (quad *)geometry;

    vector axis1 = vector_sub(self->vertices[0], self->vertices[1]);
    vector axis2 = vector_sub(self->vertices[2], self->vertices[1]);
    vector normal = vector_normalize(cross_product(axis1, axis2));

    if (!solve_linear(origin, ray, self->vertices[1], normal, &intersection))
    {
        return false;
    }

    relative_intersection = vector_sub(intersection, self->vertices[1]);
    vector orth1 = vector_orth(axis1, vector_normalize(axis2));
    vector orth2 = vector_orth(axis2, vector_normalize(axis1));
    proj1 = dot_product(relative_intersection, vector_normalize(orth1));
    proj2 = dot_product(relative_intersection, vector_normalize(orth2));
    if (.0f <= proj1 && proj1 <= vector_magnitude(orth1) &&
        .0f <= proj2 && proj2 <= vector_magnitude(orth2))
    {
        if (normal_out)
        {
            *normal_out = normal;
        }
        if (intersection_out)
        {
            *intersection_out = intersection;
        }
        return true;
    }
    else
    {
        return false;
    }
}
