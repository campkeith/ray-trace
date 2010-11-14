#include "input_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* M_PI is apparently not available in C99
   http://ubuntuforums.org/showthread.php?t=583094
*/
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef bool char_filter (char value);

static bool char_printable (char value)
{
    return !isspace(value);
}

static bool char_whitespace (char value)
{
    return isspace(value);
}

static bool vec_element (char value)
{
    return !isspace(value) && value != ',';
}

static bool tuple_begin (char value)
{
    return value == '(';
}

static bool tuple_end (char value)
{
    return value == ')';
}

static bool key_value_seperator (char value)
{
    return value == ':';
}

static bool find_next (char_filter predicate, char ** cursor)
{
    while (true)
    {
        if (**cursor == '\0')
        {
            return false;
        }
        else if (predicate(**cursor))
        {
            return true;
        }
        else
        {
            (*cursor)++;
        }
    }
}

static char * strip_comments (char * string)
{
    char * hash_ptr = strchr(string, '#');
    if (hash_ptr)
    {
        *hash_ptr = '\0';
    }
    return string;
}

static char * strip_whitespace (char * string)
{
    char * cursor = string;
    find_next(char_printable, &string);
    string = cursor;
    find_next(char_whitespace, &cursor);
    *cursor = '\0';
    return string;
}

static char * get_first_token (char ** cursor)
{
    char * token;
    if (find_next(char_printable, cursor))
    {
        token = *cursor;
        find_next(char_whitespace, cursor);
        **cursor = '\0';
        (*cursor)++;
        return token;
    }
    else
    {
        return NULL;
    }
}

static bool get_key (char ** cursor, char ** key_out)
{
    char * key;
    if (find_next(char_printable, cursor))
    {
        key = *cursor;
        if (!find_next(key_value_seperator, cursor))
        {
            fprintf(stderr, "Expected \":\" after key\n");
            return false;
        }
        **cursor = '\0';
        (*cursor)++;
        *key_out = strip_whitespace(key);
        return true;
    }
    else
    {
        return false;
    }
}

static bool parse_float (char ** cursor, float * value_out)
{
    float value;
    char * start_ptr = *cursor;
    char * end_ptr;
    value = strtof(start_ptr, &end_ptr);
    if (start_ptr == end_ptr)
    {
        fprintf(stderr, "Unable to parse floating point value.\n");
        return false;
    }
    else
    {
        *cursor = end_ptr;
        *value_out = value;
        return true;
    }
}

static bool parse_angle (char ** cursor, float * radians_out)
{
    float angle_degrees;
    if (parse_float(cursor, &angle_degrees))
    {
        *radians_out = angle_degrees / 180.0f * M_PI;
        return true;
    }
    else
    {
        return false;
    }
}

typedef bool parse_tuple_callback (char ** cursor, void * tuple, int index);

static bool parse_tuple (char ** cursor, void * tuple, int size, parse_tuple_callback parse)
{
    int index = 0;
    if (!find_next(tuple_begin, cursor))
    {
        fprintf(stderr, "Tuple expected\n");
        return false;
    }
    (*cursor)++;
    for (index = 0; index < size; index++)
    {
        find_next(vec_element, cursor);
        parse(cursor, tuple, index);
    }
    if (!find_next(tuple_end, cursor))
    {
        fprintf(stderr, "Expected close parenthesis for tuple\n");
        return false;
    }
    (*cursor)++;
    return true;
}

static bool parse_tuple_float (char ** cursor, void * tuple_ptr, int index)
{
    float * tuple = (float *)tuple_ptr;
    return parse_float(cursor, &tuple[index]);
}

static bool parse_color (char ** cursor, color * color_out)
{
    return parse_tuple(cursor, color_out, 3, parse_tuple_float);
}

static bool parse_vector (char ** cursor, vector * vector_out)
{
    return parse_tuple(cursor, vector_out, 3, parse_tuple_float);
}

static bool parse_tuple_vector (char ** cursor, void * tuple_ptr, int index)
{
    vector * tuple = (vector *)tuple_ptr;
    return parse_vector(cursor, &tuple[index]);
}

static bool parse_normal (char ** cursor, vector * normal_out)
{
    if (parse_vector(cursor, normal_out))
    {
        *normal_out = vector_normalize(*normal_out);
        return true;
    }
    else
    {
        return false;
    }
}

static bool parse_resolution (char ** cursor, int * width, int * height)
{
    float resolution[2];
    if (parse_tuple(cursor, resolution, 2, parse_tuple_float))
    {
        *width = resolution[0];
        *height = resolution[1];
        return true;
    }
    else
    {
        return false;
    }
}

static int parse_aperture (char ** cursor, aperture * aperture_out)
{
    char * key;
    while (get_key(cursor, &key))
    {
        if (strcmp(key, "position") == 0)
        {
            parse_vector(cursor, &aperture_out->position);
        }
        else if (strcmp(key, "theta") == 0)
        {
            parse_angle(cursor, &aperture_out->theta);
        }
        else if (strcmp(key, "phi") == 0)
        {
            parse_angle(cursor, &aperture_out->phi);
        }
        else if (strcmp(key, "view_angle") == 0)
        {
            parse_angle(cursor, &aperture_out->view_angle);
        }
        else if (strcmp(key, "resolution") == 0)
        {
            parse_resolution(cursor, &aperture_out->pixels_wide, &aperture_out->pixels_high);
        }
        else
        {
            fprintf(stderr, "Unknown aperture property: %s\n", key);
        }
    }
    return 0;
}

static int parse_background (char ** cursor, color * background_color_out)
{
    char * key;
    while (get_key(cursor, &key))
    {
        if (strcmp(key, "color") == 0)
        {
            parse_color(cursor, background_color_out);
        }
        else
        {
            fprintf(stderr, "Unknown background property: %s\n", key);
        }
    }
    return 0;
}

static int parse_light (char ** cursor, light_source * light_out)
{
    char * key;
    while (get_key(cursor, &key))
    {
        if (strcmp(key, "position") == 0)
        {
            parse_vector(cursor, &light_out->position);
        }
        else if (strcmp(key, "color") == 0)
        {
            parse_color(cursor, &light_out->color);
        }
        else
        {
            fprintf(stderr, "Unknown light property: %s\n", key);
        }
    }
    return 0;
}

static int handle_surface_key (char * key, char ** cursor, surface * surface_out)
{
    if (strcmp(key, "specular") == 0)
    {
        parse_color(cursor, &surface_out->specular_part);
    }
    else if (strcmp(key, "diffuse") == 0)
    {
        parse_color(cursor, &surface_out->diffuse_part);
    }
    else if (strcmp(key, "refraction_index") == 0)
    {
        parse_float(cursor, &surface_out->refraction_index);
    }
    else
    {
        return 1;
    }
    return 0;
}

static int parse_sphere (char ** cursor, surface * surface_out)
{
    char * key;
    sphere * cur_sphere = (sphere *)surface_out->user_data;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out) == 0)
        {
            continue;
        }
        else if (strcmp(key, "center") == 0)
        {
            parse_vector(cursor, &cur_sphere->center);
        }
        else if (strcmp(key, "radius") == 0)
        {
            parse_float(cursor, &cur_sphere->radius);
        }
        else 
        {
            fprintf(stderr, "Unknown sphere property: %s\n", key);
        }
    }
    surface_out->type = SURFACE_SPHERE;
    surface_out->calculate_intersection = sphere_intersect;
    return 0;
}

static int parse_frustum (char ** cursor, surface * surface_out)
{
    char * key;
    frustum * cur_frustum = (frustum *)surface_out->user_data;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out) == 0)
        {
            continue;
        }
        else if (strcmp(key, "centers") == 0)
        {
            parse_tuple(cursor, cur_frustum->centers, 2, parse_tuple_vector);
        }
        else if (strcmp(key, "radii") == 0)
        {
            parse_tuple(cursor, cur_frustum->radii, 2, parse_tuple_float);
        }
    }
    surface_out->type = SURFACE_FRUSTUM;
    surface_out->calculate_intersection = frustum_intersect;
    return 0;
}

static int parse_circle (char ** cursor, surface * surface_out)
{
    char * key;
    circle * cur_circle = (circle *)surface_out->user_data;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out) == 0)
        {
            continue;
        }
        else if (strcmp(key, "center") == 0)
        {
            parse_vector(cursor, &cur_circle->center);
        }
        else if (strcmp(key, "normal") == 0)
        {
            parse_normal(cursor, &cur_circle->normal);
        }
        else if (strcmp(key, "radius") == 0)
        {
            parse_float(cursor, &cur_circle->radius);
        }
        else
        {
            fprintf(stderr, "Unknown circle property: %s\n", key);
        }
    }
    surface_out->type = SURFACE_CIRCLE;
    surface_out->calculate_intersection = circle_intersect;
    return 0;
}

static int parse_quad (char ** cursor, surface * surface_out)
{
    char * key;
    quad * cur_quad = (quad *)surface_out->user_data;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out) == 0)
        {
            continue;
        }
        else if (strcmp(key, "vertices") == 0)
        {
            parse_tuple(cursor, cur_quad->vertices, 3, parse_tuple_vector);
        }
        else
        {
            fprintf(stderr, "Unknown quad property: %s\n", key);
        }
    }
    surface_out->type = SURFACE_QUAD;
    surface_out->calculate_intersection = quad_intersect;
    return 0;
}

int load_scene (FILE * file, scene * scene_out)
{
    const int buf_size = 256;
    char buffer[buf_size];
    char * object_name;
    char * cursor;
    int line = 0;

    light_source * cur_light;
    surface * cur_surface;

    cur_light = scene_out->light_sources = calloc(16, sizeof(light_source));
    cur_surface = scene_out->surfaces = calloc(16, sizeof(surface));

    while (fgets(buffer, buf_size, file))
    {
        strip_comments(buffer);
        line++;
        cursor = buffer;
        object_name = get_first_token(&cursor);
        if (object_name == NULL)
        {
            continue;
        }

        if (strcmp(object_name, "aperture") == 0)
        {
            parse_aperture(&cursor, &scene_out->aperture);
        }
        else if (strcmp(object_name, "background") == 0)
        {
            parse_background(&cursor, &scene_out->background_color);
        }
        else if (strcmp(object_name, "light") == 0)
        {
            parse_light(&cursor, cur_light);
            cur_light++;
        }
        else if (strcmp(object_name, "sphere") == 0)
        {
            cur_surface->user_data = calloc(1, sizeof(sphere));
            parse_sphere(&cursor, cur_surface);
            cur_surface++;
        }
        else if (strcmp(object_name, "frustum") == 0)
        {
            cur_surface->user_data = calloc(1, sizeof(frustum));
            parse_frustum(&cursor, cur_surface);
            cur_surface++;
        }
        else if (strcmp(object_name, "circle") == 0)
        {
            cur_surface->user_data = calloc(1, sizeof(circle));
            parse_circle(&cursor, cur_surface);
            cur_surface++;
        }
        else if (strcmp(object_name, "quad") == 0)
        {
            cur_surface->user_data = calloc(1, sizeof(quad));
            parse_quad(&cursor, cur_surface);
            cur_surface++;
        }
        else
        {
            fprintf(stderr, "Line %d: Unknown object type: \"%s\"", line, object_name);
            return -1;
        }
    }

    cur_light->type = LIGHT_SOURCE_SENTINEL;
    cur_surface->type = SURFACE_SENTINEL;
    return 0;
}
