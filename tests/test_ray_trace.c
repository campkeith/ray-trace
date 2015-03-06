#include "scene.h"

#include <stdio.h>

surface * hit_surface (vector origin, vector ray, surface surfaces[],
                       vector * intersection_out, vector * normal_out);
bool is_illuminated (light_source * source, vector point, surface surfaces[]);
color get_illumination (vector point, vector ray, vector normal,
                        light_source light_sources[], surface surfaces[]);
color cast_ray (scene * scene, vector origin, vector ray, int depth);

static int tests_run;
static int tests_passed;

bool approx_equal (float expected, float actual)
{
    const float max_error = 1e-4;
    if (actual < max_error)
    {
        return fabs(expected - actual) < max_error;
    }
    else
    {
        float relative_error = fabs(expected - actual) / fabs(actual);
        return relative_error < max_error;
    }
}

void test_pointer (char * label, void * expected, void * actual)
{
    if (expected == actual)
    {
        printf("Pass: %s: %p = %p\n", label, expected, actual);
        tests_passed++;
    }
    else
    {
        printf("Fail: %s: expected %p, got %p\n", label, expected, actual);
    }
    tests_run++;
}

char * strbool (bool value)
{
    return value ? "true" : "false";
}

void test_bool (char * label, bool expected, bool actual)
{
    if (expected == actual)
    {
        printf("Pass: %s: %s = %s\n", label, strbool(expected), strbool(actual));
        tests_passed++;
    }
    else
    {
        printf("Fail: %s: expected %s, got %s\n", label, strbool(expected), strbool(actual));
    }
    tests_run++;
}

void test_float (char * label, float expected, float actual)
{
    if (approx_equal(expected, actual))
    {
        printf("Pass: %s: %f ~= %f\n", label, expected, actual);
        tests_passed++;
    }
    else
    {
        printf("Fail: %s: expected %f, got %f\n", label, expected, actual);
    }
    tests_run++;
}

void test_vector (char * label, vector expected, vector actual)
{
    if (approx_equal(expected.x, actual.x) &&
        approx_equal(expected.y, actual.y) &&
        approx_equal(expected.z, actual.z))
    {
        printf("Pass: %s: (x:%f, y:%f, z:%f) ~= (x:%f, y:%f, z:%f)\n", label,
               expected.x, expected.y, expected.z, actual.x, actual.y, actual.z);
        tests_passed++;
    }
    else
    {
        printf("Fail: %s: expected (x:%f, y:%f, z:%f), got (x:%f, y:%f, z:%f)\n", label,
               expected.x, expected.y, expected.z, actual.x, actual.y, actual.z);
    }
    tests_run++;
}

void test_color (char * label, color expected, color actual)
{
    if (approx_equal(expected.r, actual.r) &&
        approx_equal(expected.g, actual.g) &&
        approx_equal(expected.b, actual.b))
    {
        printf("Pass: %s: (r:%f, g:%f, b:%f) ~= (r:%f, g:%f, b:%f)\n", label,
               expected.r, expected.g, expected.b, actual.r, actual.g, actual.b);
        tests_passed++;
    }
    else
    {
        printf("Fail: %s: expected (r:%f, g:%f, b:%f), got (r:%f, g:%f, b:%f)\n", label,
               expected.r, expected.g, expected.b, actual.r, actual.g, actual.b);
    }
    tests_run++;
}

void test_hit_surface ()
{
    surface surfaces[] = {{.class = surface_circle}, {.class = surface_sphere},
                          {.class = surface_circle}, {.class = NULL}};
    *(circle *)surfaces[0].geometry = (circle){ .center = { 0,0,0}, .radius = 5, .normal = {1,0,0} };
    *(sphere *)surfaces[1].geometry = (sphere){ .center =  {0,0,0}, .radius = 3 };
    *(circle *)surfaces[2].geometry = (circle){ .center = {-1,0,0}, .radius = 2, .normal = {-1,0,0} };

    vector origin = { -5, 0, 0};
    vector direction = { 1, 0, 0};
    vector intersection, normal;
    surface * result = hit_surface(origin, direction, surfaces, &intersection, &normal);

    test_pointer("hit_surface result 1", &surfaces[1], result);
    test_vector("hit_surface normal", (vector){-1,0,0}, normal);
    test_vector("hit_surface intersection", (vector){-3,0,0}, intersection);

    origin = (vector){ 0, 4, 6 };
    direction = (vector){ 0, 1, 0};
    result = hit_surface(origin, direction, surfaces, &intersection, &normal);
    
    test_pointer("hit_surface result 2", NULL, result);
}

void test_is_illuminated ()
{
    light_source lights[] = {{.position = {-2,2,0}}, {.position = {2,2,0}}, {.position = {0,-2,0}}};

    surface surfaces[] = {{.class = surface_circle}, {.class = surface_circle}, {.class = NULL}};
    *(circle *)surfaces[0].geometry = (circle){ .center = {0,0,0}, .radius = 1, .normal = {1,0,0} };
    *(circle *)surfaces[1].geometry = (circle){ .center = {0,0,0}, .radius = 1, .normal = {0,1,0} };
    
    vector point = {0.5,0,0};
    
    test_bool("is_illuminated test 1", false, is_illuminated(&lights[0], point, surfaces));
    test_bool("is_illuminated test 2", true, is_illuminated(&lights[1], point, surfaces));
    test_bool("is_illuminated test 3", true, is_illuminated(&lights[2], point, surfaces));
}

void test_get_illumination ()
{
    light_source lights[] = { {.position = {-2,2,0}, .color = {1.0, 0, 0}},
                              {.position = {2,2,0},  .color = {0, 0, 1.0}},
                              {.position = {0,-2,0}, .color = {1.0,1.0,0}},
                              {.type = LIGHT_SOURCE_SENTINEL}};

    surface surfaces[] = {{.class = surface_circle}, {.class = surface_circle}, {.class = NULL}};
    *(circle *)surfaces[0].geometry = (circle){ .center = {0,0,0}, .radius = 1, .normal = {1,0,0} };
    *(circle *)surfaces[1].geometry = (circle){ .center = {0,0,0}, .radius = 1, .normal = {0,1,0} };

    vector point = {0.5,0,0};
    vector ray = {1.0/3.0, 1.0/4.0, 0};
    vector normal = {0, -1, 0};

    color result = get_illumination(point, ray, normal, lights, surfaces);
    test_color("get_illumination test 1", (color){0.970143,0.970143,0}, result);

    point = (vector){-0.5,0,0};
    ray = (vector){0,-1,0}; 
    normal = (vector){0,-1,0};
    result = get_illumination(point, ray, normal, lights, surfaces);
    test_color("get_illumination test 2", (color){0.8,0,0}, result);
}

void test_cast_ray ()
{
    color result;
    light_source lights[] = { {.position = {-2,2,0}, .color = {1.0, 0, 0}},
                              {.position = {2,2,0},  .color = {0, 0, 1.0}},
                              {.position = {1,-2,0}, .color = {1.0,1.0,0}},
                              {.type = LIGHT_SOURCE_SENTINEL}};

    surface surfaces[] = {{.class = surface_circle, .specular_part = {0.4,0.4,0.6}, .diffuse_part = {0.1,0.1,0.2}},
                          {.class = surface_circle, .specular_part = {0.3,0.8,0.3}, .refraction_index = 0.8},
                          {.class = NULL}};
    *(circle *)surfaces[0].geometry = (circle){ .center = {0,0,0}, .radius = 2, .normal = {1,0,0} };
    *(circle *)surfaces[1].geometry = (circle){ .center = {0,0,0}, .radius = 2, .normal = {0,1,0} };

    scene cur_scene = {.background_color = {0.5, 0, 0.5}, .light_sources = lights, .surfaces = surfaces};
    result = cast_ray(&cur_scene, (vector){2,-2,0}, (vector){-1.0/4.0, 1.0/3.0, 0}, 0);
    test_color("cast_ray depth 0", cur_scene.background_color, result);
    result = cast_ray(&cur_scene, (vector){2,-2,0}, (vector){1.0/4.0, 1.0/3.0, 0}, 8);
    test_color("cast_ray to background", cur_scene.background_color, result);
    result = cast_ray(&cur_scene, (vector){-2,-2,0}, (vector){1.0/3.0, 1.0/4.0, 0}, 8);
    test_color("cast_ray to surfaces 1", (color){0.06, 0, 0.090000}, result);
    result = cast_ray(&cur_scene, (vector){2,-2,0}, (vector){-1.0/4.0, 1.0/3.0, 0}, 8);
    test_color("cast_ray to surfaces 2", (color){0.062055, 0.005480, 0.142316}, result);
}

int main ()
{
    tests_run = tests_passed = 0;
    
    test_hit_surface();
    test_is_illuminated();
    test_get_illumination();
    test_cast_ray();

    printf("%d out of %d tests passed.\n", tests_passed, tests_run);

    if (tests_passed == tests_run)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
