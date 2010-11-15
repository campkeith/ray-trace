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

/* Input file parser implementation.  For input file examples, refer to the
   scenes directory.

   This module's defines the load_scene function which reads a given input file
   stream and populates an output "scene" data structure, dynamically allocating
   memory for array data as needed.  The data structure is defined and
   documented in scene.h


   =====================================================================================
   Input file format
   =====================================================================================

   The input file to the ray tracer is first divided into a set of newline
   character seperated lines, each line being parsed independently.  The
   "#" character is used for comments.  The "#" and any characters after
   it up to a newline character are ignored.  Empty or blank lines are ignored.

   Each non-blank line declares an object and defines its properties with the following
   general format.

   <object_name> <property_name>:<property_value> <property_name>:<property_value> ...

   The following object names are allowed:

   "aperture", "light", "sphere", "frustum", "circle", "quad"

   Each object has a set of allowed properties:

   aperture: "position", "direction", "view_angle", "resolution"
   light:    "position", "color"
   sphere:   "center", "radius"
   frustum:  "centers", "radii"
   circle:   "center", "radius", "normal"
   quad:     "vertices"

   "sphere", "frustum", "circle", and "quad" objects have additional surface properties.
   surface properties: "diffuse", "specular", "refraction_index"

   Properties can be declared in any order, but will not be repeated.  Properties
   can be absent, in which case a default value of 0 is used for data associated
   with that property.

   The format of the value associated with each property is property dependent.
   They are defined as follows:

   position, center, normal: <vector>
   direction: <2-tuple of decimals> (theta, phi)
   resolution: <2-tuple of decimals> (pixels wide, pixels high)
   color, diffuse, specular: <RGB>
   radius, refraction_index, view_angle: <decimal>
   centers: <2-tuple of vectors>
   vertices: <3-tuple of vectors>
   radii: <2-tuple of decimals>

   The decimal values associated with "direction", and "view_angle"
   are angles given in degrees.  These must be converted to radians so that
   they are compatible with the math library.

   The decimal values associated with "resolution" will not have fractional
   parts.  These values must be converted to integer data types.

   The vector associated with "normal" must be normalized with the
   vector_normalize function so that it is compatible with the ray tracing
   engine.

   An n-tuple is a set of homogenous values (like an array).  All
   values within a given tuple have the same format.
   The format for the elements in each tuple are specified above.

   A 2-tuple has this format:
   (<value1>,<value2>)

   A 3-tuple has this format:
   (<value1>,<value2>,<value3>)

   A "vector" is a 3-tuple of decimals where the decimal values are interpreted
   as the x,y,z components of the vector.

   An "RGB" is a 3-tuple of decimals where the decimal values are interpreted
   the red, green, blue components of a color.  For each color component, 0
   is minimum intensity/transmittance while 1 is maximum intensity/transmittance.

   A "decimal" is a decimal number as defined and interpreted by the strtof function:
   http://linux.die.net/man/3/strtof


   =====================================================================================
   Implementation notes
   =====================================================================================
   
   The parsing problem is systematically decomposed into a series of parsing functions
   in a way that mimickes the hierchical description of the input file format given above.
   
   Most parsing functions looks like this:

   bool parse (char ** cursor, foo * foo_out);

   The cursor double pointer is both an input and an output.  As an input, the cursor
   points to a pointer to the beginning location of a char array to be parsed.
   As an output, the dereferenced cursor points to the next character after the item
   that was parsed.  Once parsing is complete, the dereferenced foo_out parameter
   is also filled in with the information parsed.  Thus the cursor is used to
   keep track of what has been parsed, and what remains to be parsed.

   Parsing functions also have return values which indicate if a parse error has occurred.
   For the purposes of this project, handling parse errors is not a concern.
*/

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

static char * get_next_word (char ** cursor)
{
    char * word;
    if (find_next(char_printable, cursor))
    {
        word = *cursor;
        find_next(char_whitespace, cursor);
        **cursor = '\0';
        (*cursor)++;
        return word;
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
        	**cursor = '\0';
            fprintf(stderr, "Expected \":\" after key \"%s\"", key);
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
/*! Student implementation candidate */
{
    float value;
    char * start_ptr = *cursor;
    char * end_ptr;
    value = strtof(start_ptr, &end_ptr);
    if (start_ptr == end_ptr)
    {
        fprintf(stderr, "Unable to parse floating point value: \"%c\"\n", **cursor);
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
/*! Student implementation candidate */
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
        fprintf(stderr, "Tuple expected, but not found\n");
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

static bool parse_tuple_angle (char ** cursor, void * tuple_ptr, int index)
{
    float * tuple = (float *)tuple_ptr;
    return parse_angle(cursor, &tuple[index]);
}

static bool parse_color (char ** cursor, color * color_out)
/*! Student implementation candidate */
{
    return parse_tuple(cursor, color_out, 3, parse_tuple_float);
}

static bool parse_vector (char ** cursor, vector * vector_out)
/*! Student implementation candidate */
{
    return parse_tuple(cursor, vector_out, 3, parse_tuple_float);
}

static bool parse_tuple_vector (char ** cursor, void * tuple_ptr, int index)
{
    vector * tuple = (vector *)tuple_ptr;
    return parse_vector(cursor, &tuple[index]);
}

static bool parse_normal (char ** cursor, vector * normal_out)
/*! Student implementation candidate */
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

static bool parse_resolution (char ** cursor, resolution * resolution_out)
/*! Student implementation candidate */
{
    float resolution[2];
    if (parse_tuple(cursor, resolution, 2, parse_tuple_float))
    {
        resolution_out->width = resolution[0];
        resolution_out->height = resolution[1];
        return true;
    }
    else
    {
        return false;
    }
}

static bool parse_direction (char ** cursor, direction * direction_out)
/*! Student implementation candidate */
{
    return parse_tuple(cursor, direction_out, 2, parse_tuple_angle);
}

static int parse_aperture (char ** cursor, aperture * aperture_out)
/*! Student implementation candidate */
{
    char * key;
    while (get_key(cursor, &key))
    {
        if (strcmp(key, "position") == 0)
        {
            parse_vector(cursor, &aperture_out->position);
        }
        else if (strcmp(key, "direction") == 0)
        {
            parse_direction(cursor, &aperture_out->direction);
        }
        else if (strcmp(key, "resolution") == 0)
        {
            parse_resolution(cursor, &aperture_out->resolution);
        }
        else if (strcmp(key, "view_angle") == 0)
        {
            parse_angle(cursor, &aperture_out->view_angle);
        }
        else
        {
            fprintf(stderr, "Unknown aperture property: %s\n", key);
        }
    }
    return 0;
}

static int parse_background (char ** cursor, color * background_color_out)
/*! Student implementation candidate */
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
/*! Student implementation candidate */
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

static bool handle_surface_key (char * key, char ** cursor, surface * surface_out)
/* Helper function, could be inlined or kept */
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
        return false;
    }
    return true;
}

static int parse_sphere (char ** cursor, surface * surface_out)
/* Given to provide a useful example */
{
    char * key;
    sphere * cur_sphere = (sphere *)surface_out->geometry;
    surface_out->class = surface_sphere;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out))
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
    return 0;
}

static int parse_frustum (char ** cursor, surface * surface_out)
/*! Student implementation candidate */
{
    char * key;
    frustum * cur_frustum = (frustum *)surface_out->geometry;
    surface_out->class = surface_frustum;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out))
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
        else
        {
        	fprintf(stderr, "Unknown frustum property: %s\n", key);
        }
    }
    return 0;
}

static int parse_circle (char ** cursor, surface * surface_out)
/*! Student implementation candidate */
{
    char * key;
    circle * cur_circle = (circle *)surface_out->geometry;
    surface_out->class = surface_circle;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out))
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
    return 0;
}

static int parse_quad (char ** cursor, surface * surface_out)
/*! Student implementation candidate */
{
    char * key;
    quad * cur_quad = (quad *)surface_out->geometry;
    surface_out->class = surface_quad;
    while (get_key(cursor, &key))
    {
        if (handle_surface_key(key, cursor, surface_out))
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
    return 0;
}

int load_scene (FILE * file, scene * scene_out)
/* Given to provide the high level structure */
{
    const int max_light_sources = 256;
    const int max_surfaces = 256;
    const int buf_size = 256;
    char buffer[buf_size];
    char * object_name;
    char * cursor;
    int line = 0;

    light_source * cur_light;
    surface * cur_surface;

    cur_light = scene_out->light_sources = calloc(max_light_sources, sizeof(light_source));
    cur_surface = scene_out->surfaces = calloc(max_surfaces, sizeof(surface));

    while (fgets(buffer, buf_size, file))
    {
        line++;
        strip_comments(buffer);
        cursor = buffer;
        object_name = get_next_word(&cursor);
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
            parse_sphere(&cursor, cur_surface);
            cur_surface++;
        }
        else if (strcmp(object_name, "frustum") == 0)
        {
            parse_frustum(&cursor, cur_surface);
            cur_surface++;
        }
        else if (strcmp(object_name, "circle") == 0)
        {
            parse_circle(&cursor, cur_surface);
            cur_surface++;
        }
        else if (strcmp(object_name, "quad") == 0)
        {
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
    cur_surface->class = NULL;
    return 0;
}
