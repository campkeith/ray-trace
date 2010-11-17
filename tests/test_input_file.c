#include "scene.h"

#include <stdio.h>
#ifndef __USE_XOPEN
    #define __USE_XOPEN
#endif
#include <math.h>

bool parse_angle (char ** cursor, float * radians_out);
bool parse_vector (char ** cursor, vector * vector_out);
bool parse_normal (char ** cursor, vector * normal_out);
bool parse_resolution (char ** cursor, resolution * resolution_out);
int parse_camera (char ** cursor, camera * camera_out);
int parse_background (char ** cursor, color * background_color_out);
int parse_frustum (char ** cursor, surface * surface_out);
int parse_circle (char ** cursor, surface * surface_out);

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

void test_resolution (char * label, resolution expected, resolution actual)
{
    if (expected.width == actual.width && expected.height == actual.height)
    {
        printf("Pass: %s: (width:%d, height:%d) = (width:%d, height:%d)\n",
               label, expected.width, expected.height, actual.width, actual.height);
        tests_passed++;
    }
    else
    {
        printf("Fail: %s: expected (width:%d, height:%d), got (width:%d, height:%d)\n",
               label, expected.width, expected.height, actual.width, actual.height);
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

void test_parse_angle ()
{
    char string1[] = "(60.0, 40)";
    char string2[] = "-30";
    char * cursor = &string1[1];
    float radians;
    parse_angle(&cursor, &radians);
    test_float("Angle test 1", 2.0f * M_PI / 6.0f, radians);
    
    cursor = string2;
    parse_angle(&cursor, &radians);
    test_float("Angle test 2", -M_PI / 6.0f, radians);
}

void test_parse_vector ()
{
    char string1[] = "(1,2,3)";
    char string2[] = "(-13,2e-4,0.0)";
    char * cursor = string1;
    vector result;
    parse_vector(&cursor, &result);
    test_vector("Vector test 1", (vector){1, 2, 3}, result);
    
    cursor = string2;
    parse_vector(&cursor, &result);
    test_vector("Vector test 2", (vector){-13, 2e-4f, 0.0}, result);
}

void test_parse_normal ()
{
    char string1[] = "(0, -1, 0)";
    char string2[] = "(1, -1, 0)";
    
    char * cursor = string1;
    vector result;
    parse_normal(&cursor, &result);
    test_vector("Normal test 1", (vector){0, -1, 0}, result);

    cursor = string2;
    parse_normal(&cursor, &result);
    test_vector("Normal test 2", (vector){M_SQRT1_2, -M_SQRT1_2, 0}, result);
}

void test_parse_resolution ()
{
    char string[] = "(640, 480)";
    char * cursor = string;
    resolution result;
    resolution expected = {640, 480};
    
    parse_resolution(&cursor, &result);
    test_resolution("Resolution test", expected, result);
}

void test_parse_camera ()
{
    char string[] = "camera position:(1,2,3) direction:(45, -20) view_angle:90 resolution:(800,600)";
    char * cursor = &string[6];
    camera result;

    parse_camera(&cursor, &result);

    test_vector("Camera position", (vector){1,2,3}, result.position);
    test_float("Camera direction theta", M_PI / 4.0f, result.direction.theta);
    test_float("Camera direction phi", -M_PI / 9.0f, result.direction.phi);
    test_float("Camera view angle", M_PI / 2.0f, result.view_angle);
    test_resolution("Camera resolution", (resolution){800, 600}, result.resolution);
}

void test_parse_background ()
{
    char string[] = "background color:(0.0, 0.4, 1.0)";
    char * cursor = &string[10];
    color result;
    
    parse_background(&cursor, &result);

    test_color("Background color", (color){0.0, 0.4, 1.0}, result);
}

void test_parse_frustum ()
{
    char string[] = "frustum centers:((-10, 0, 10), (20, 10, 10)) radii:(0, 30) "
                    "diffuse:(0.0, 0.9, 0.4) specular:(0.0, 0.1, 0.05) refraction_index:1.2";
    char * cursor = &string[7];
    surface result;
    frustum * geometry = (frustum *)&result.geometry;
    
    parse_frustum(&cursor, &result);
    
    test_vector("Frustum center 0", (vector){-10,  0, 10}, geometry->centers[0]);
    test_vector("Frustum center 1", (vector){ 20, 10, 10}, geometry->centers[1]);
    test_float("Frustum radius 0", 0, geometry->radii[0]);
    test_float("Frustum radius 1", 30, geometry->radii[1]);
    test_color("Frustum diffuse color", (color){0.0, 0.9, 0.4}, result.diffuse_part);
    test_color("Frustum specular color", (color){0.0, 0.1, 0.05}, result.specular_part);
    test_float("Frustum refraction index", 1.2, result.refraction_index);
}

void test_parse_circle ()
{
    char string[] = "circle center:(0, 1, 0) radius:40 normal:(0, 0, -4) "
                    "diffuse:(0.4, 0.0, 0.0) specular:(0.6, 0.0, 0.0) refraction_index:1.0";
    char * cursor = &string[6];
    surface result;
    circle * geometry = (circle *)&result.geometry;

    parse_circle(&cursor, &result);
    
    test_vector("Circle center", (vector){0, 1, 0}, geometry->center);
    test_vector("Circle normal", (vector){0, 0, -1}, geometry->normal);
    test_float("Circle radius", 40, geometry->radius);
    test_color("Circle diffuse color", (color){0.4, 0.0, 0.0}, result.diffuse_part);
    test_color("Circle specular color", (color){0.6, 0.0, 0.0}, result.specular_part);
    test_float("Circle refraction index", 1.0, result.refraction_index);
}

int main ()
{
    tests_run = tests_passed = 0;
    
    test_parse_angle();
    test_parse_vector();
    test_parse_normal();
    test_parse_resolution();
    test_parse_camera();
    test_parse_background();
    test_parse_frustum();
    test_parse_circle();
    
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
