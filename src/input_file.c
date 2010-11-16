#include "input_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* Define __USE_XOPEN to get M_PI and friends from math.h.
   Apparently ANSI C99 doesn't provide these constants:
   http://ubuntuforums.org/showthread.php?t=583094
*/
#ifndef __USE_XOPEN
    #define __USE_XOPEN
#endif
#include <math.h>

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

   "camera", "light", "sphere", "frustum", "circle", "quad"

   Each object has a set of allowed properties:

   camera: "position", "direction", "view_angle", "resolution"
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
   color, diffuse, specular: <color>
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

   An "color" is a 3-tuple of decimals where the decimal values are interpreted
   the red, green, blue components of a color.  For each color component, 0
   is minimum intensity/transmittance while 1 is maximum intensity/transmittance.

   A "decimal" is a decimal number as defined and interpreted by the strtof function:
   http://linux.die.net/man/3/strtof


   =====================================================================================
   Implementation notes
   =====================================================================================
   
   The parsing problem is systematically decomposed into a series of parsing functions
   in a way that mimics the hierchical description of the input file format given above.
   
   Most parsing functions looks like this:

   bool parse (char ** cursor, foo * foo_out);

   The cursor double pointer is both an input and an output.  As an input, the cursor
   points to a pointer to the beginning location of a char array to be parsed.
   As an output, the dereferenced cursor points to the next character after the item
   that was parsed.  Once parsing is complete, the dereferenced foo_out parameter
   is also filled in with the information parsed.  Thus the cursor is used to
   keep track of what has been parsed, and what remains to be parsed.
   
   Parsing is destructive, meaning that the string being parsed is altered as the cursor
   runs over it.  The parsing algorithm is like cutting pieces of a tape.  The string
   is "cut" after each name or value is parsed by the insertion of null sentinels.
   A reference to the newly cut string is returned from the low-level parsing function
   while the cursor is advanced to point to the rest of the "tape".

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

static bool vector_element (char value)
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

static bool property_value_seperator (char value)
{
    return value == ':';
}

static bool find_next (char_filter predicate, char ** cursor)
/*! Advance the cursor to the next character that satisfies the given
    predicate, which is one of the predicate functions defined above.

    Return true if a desired character is found, false if the end of
    the string is reached and a desired character is not found.
*/
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
/*! Strip a '#' character and any characters that follow by
    cutting at the first '#'
*/
{
    char * hash_ptr = strchr(string, '#');
    if (hash_ptr)
    {
        *hash_ptr = '\0';
    }
    return string;
}

static char * strip_whitespace (char * string)
/*! Strip leading and trailing whitespace in "string" by cutting
    off trailing whitespace, and returning a pointer to the
    first printable character
*/
{
    char * cursor = string;
    find_next(char_printable, &string);
    string = cursor;
    find_next(char_whitespace, &cursor);
    *cursor = '\0';
    return string;
}

static char * get_next_word (char ** cursor)
/*! Find the next word (sequence of printable character) in the
    string, cut it from the string, advance the cursor, and
    return the word
*/
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

static bool get_next_property (char ** cursor, char ** property_out)
/*! Get the next property name in the string, assumming the following format:
    <property_name>:<property_value>
    Once found, cut the property name from the string, advance the cursor
    to the property value, and return the property name.
*/
{
    char * property;
    if (find_next(char_printable, cursor))
    {
        property = *cursor;
        if (!find_next(property_value_seperator, cursor))
        {
        	**cursor = '\0';
            fprintf(stderr, "Expected \":\" after property \"%s\"", property);
            return false;
        }
        **cursor = '\0';
        (*cursor)++;
        *property_out = strip_whitespace(property);
        return true;
    }
    else
    {
        return false;
    }
}

static bool parse_float (char ** cursor, float * value_out)
/*! Parse a decimal value using strtof: http://linux.die.net/man/3/strtof
    Output the floating point value parsed to "value_out" and advance
    "cursor" to the next character after the value parsed.
*/
{
    *value_out = strtof(*cursor, cursor);
    return true;
}

static bool parse_angle (char ** cursor, float * radians_out)
/*! Parse a decimal value and interpret it as an angle in degrees.
    Output the value in radians to "radians_out" and advance "cursor" to
    the next character after the value parsed.

    The M_PI macro will be useful for the degrees to radians conversion:
    http://linux.die.net/man/3/m_pi
*/
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

static bool parse_tuple_float (char ** cursor, float tuple_out[], int size)
/*! Parse an <n-tuple of decimal> as defined in the input file format with n
    being "size".  Output array "tuple_out" is populated with the floating
    point values of the tuple elements, and the cursor is advanced to the next
    character after the end of the tuple.
*/
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
        find_next(vector_element, cursor);
        parse_float(cursor, &tuple_out[index]);
    }
    if (!find_next(tuple_end, cursor))
    {
        fprintf(stderr, "Expected close parenthesis for tuple\n");
        return false;
    }
    (*cursor)++;
    return true;
}

static bool parse_tuple_angle (char ** cursor, float tuple_out[], int size)
/*! Parse an <n-tuple of decimal> as defined in the input file format and
    interpret the decimal values as angles in degrees.  Populate "tuple_out"
    with the angles in radians and advance the cursor.
*/
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
        find_next(vector_element, cursor);
        parse_angle(cursor, &tuple_out[index]);
    }
    if (!find_next(tuple_end, cursor))
    {
        fprintf(stderr, "Expected close parenthesis for tuple\n");
        return false;
    }
    (*cursor)++;
    return true;
}

static bool parse_color (char ** cursor, color * color_out)
/*! Parse a <color> as defined in the input file format above.
    Output the color through the "color_out" parameter and advance
    the cursor to the next character after the color tuple.
*/
{
    return parse_tuple_float(cursor, (float *)color_out, 3);
}

static bool parse_vector (char ** cursor, vector * vector_out)
/*! Parse a <vector> as defined in the input file format above.
    Output the vector through the "vector_out" parameter and advance
    the cursor to the next character after the vector tuple.
*/
{
    return parse_tuple_float(cursor, (float *)vector_out, 3);
}

static bool parse_normal (char ** cursor, vector * normal_out)
/*! Parse a <vector> as defined in the input file format,
    normalize it with the vector_normalize function, (defined in vector.h)
    output the normalized vector to the "normal_out" parameter,
    and advance the cursor.
*/
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

static bool parse_tuple_vector (char ** cursor, vector tuple_out[], int size)
/*! Parse an <n-tuple of vector> as defined in the input file format with n
    being "size".  Output array "tuple_out" is populated with the result of
    parsing the vector tuple elements, and the cursor is advanced to the next
    character after the end of the tuple.
*/
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
        find_next(vector_element, cursor);
        parse_vector(cursor, &tuple_out[index]);
    }
    if (!find_next(tuple_end, cursor))
    {
        fprintf(stderr, "Expected close parenthesis for tuple\n");
        return false;
    }
    (*cursor)++;
    return true;
}

static bool parse_resolution (char ** cursor, resolution * resolution_out)
/*! Parse a <resolution> as defined in the input file format,
    Convert the components to integers, output in "resolution_out",
    and advance the cursor.
*/
{
    float resolution[2];
    if (parse_tuple_float(cursor, resolution, 2))
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
/*! Parse a <direction> as defined in the input file format,
    output in "direction_out", and advance the cursor.
*/
{
    return parse_tuple_angle(cursor, (float *)direction_out, 2);
}

static int parse_camera (char ** cursor, camera * camera_out)
/*! Parse an <camera>, output it to "camera_out", advance the cursor.
    Note that the camera data has already been zeroed; there is no
    need to initialize the struct members */
{
    char * property;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "position") == 0)
        {
            parse_vector(cursor, &camera_out->position);
        }
        else if (strcmp(property, "direction") == 0)
        {
            parse_direction(cursor, &camera_out->direction);
        }
        else if (strcmp(property, "resolution") == 0)
        {
            parse_resolution(cursor, &camera_out->resolution);
        }
        else if (strcmp(property, "view_angle") == 0)
        {
            parse_angle(cursor, &camera_out->view_angle);
        }
        else
        {
            fprintf(stderr, "Unknown camera property: %s\n", property);
        }
    }
    return 0;
}

static int parse_background (char ** cursor, color * background_color_out)
/*! Parse a <background>, output its background color to "background_color_out",
    advance the cursor */
{
    char * property;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "color") == 0)
        {
            parse_color(cursor, background_color_out);
        }
        else
        {
            fprintf(stderr, "Unknown background property: %s\n", property);
        }
    }
    return 0;
}

static int parse_light (char ** cursor, light_source * light_out)
/*! Parse a <light>, output it to "light_out", advance the cursor. */
{
    char * property;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "position") == 0)
        {
            parse_vector(cursor, &light_out->position);
        }
        else if (strcmp(property, "color") == 0)
        {
            parse_color(cursor, &light_out->color);
        }
        else
        {
            fprintf(stderr, "Unknown light property: %s\n", property);
        }
    }
    return 0;
}

/* For the all surface parsing functions, the surface data
   passed will be zeroed, so there is no need to initialize
   default values that are zero */

static int parse_sphere (char ** cursor, surface * surface_out)
/*! Parse a <sphere>, populating the members of "surface_out" to
    represent a sphere as specified in "surface.h", advance cursor.
*/
{
    char * property;
    sphere * cur_sphere = (sphere *)surface_out->geometry;
    surface_out->class = surface_sphere;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "specular") == 0)
        {
            parse_color(cursor, &surface_out->specular_part);
        }
        else if (strcmp(property, "diffuse") == 0)
        {
            parse_color(cursor, &surface_out->diffuse_part);
        }
        else if (strcmp(property, "refraction_index") == 0)
        {
            parse_float(cursor, &surface_out->refraction_index);
        }
        else if (strcmp(property, "center") == 0)
        {
            parse_vector(cursor, &cur_sphere->center);
        }
        else if (strcmp(property, "radius") == 0)
        {
            parse_float(cursor, &cur_sphere->radius);
        }
        else 
        {
            fprintf(stderr, "Unknown sphere property: %s\n", property);
        }
    }
    return 0;
}

static int parse_frustum (char ** cursor, surface * surface_out)
/*! Parse a <frustum>, populating the members of "surface_out" to
    represent a frustum as specified in "surface.h", advance cursor.
*/
{
    char * property;
    frustum * cur_frustum = (frustum *)surface_out->geometry;
    surface_out->class = surface_frustum;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "specular") == 0)
        {
            parse_color(cursor, &surface_out->specular_part);
        }
        else if (strcmp(property, "diffuse") == 0)
        {
            parse_color(cursor, &surface_out->diffuse_part);
        }
        else if (strcmp(property, "refraction_index") == 0)
        {
            parse_float(cursor, &surface_out->refraction_index);
        }
        else if (strcmp(property, "centers") == 0)
        {
            parse_tuple_vector(cursor, cur_frustum->centers, 2);
        }
        else if (strcmp(property, "radii") == 0)
        {
            parse_tuple_float(cursor, cur_frustum->radii, 2);
        }
        else
        {
        	fprintf(stderr, "Unknown frustum property: %s\n", property);
        }
    }
    return 0;
}

static int parse_circle (char ** cursor, surface * surface_out)
/*! Parse a <circle>, populating the members of "surface_out" to
    represent a circle as specified in "surface.h", advance cursor.
*/
{
    char * property;
    circle * cur_circle = (circle *)surface_out->geometry;
    surface_out->class = surface_circle;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "specular") == 0)
        {
            parse_color(cursor, &surface_out->specular_part);
        }
        else if (strcmp(property, "diffuse") == 0)
        {
            parse_color(cursor, &surface_out->diffuse_part);
        }
        else if (strcmp(property, "refraction_index") == 0)
        {
            parse_float(cursor, &surface_out->refraction_index);
        }
        else if (strcmp(property, "center") == 0)
        {
            parse_vector(cursor, &cur_circle->center);
        }
        else if (strcmp(property, "normal") == 0)
        {
            parse_normal(cursor, &cur_circle->normal);
        }
        else if (strcmp(property, "radius") == 0)
        {
            parse_float(cursor, &cur_circle->radius);
        }
        else
        {
            fprintf(stderr, "Unknown circle property: %s\n", property);
        }
    }
    return 0;
}

static int parse_quad (char ** cursor, surface * surface_out)
/*! Parse a <quad>, populating the members of "surface_out" to
    represent a quad as specified in "surface.h", advance cursor.
*/
{
    char * property;
    quad * cur_quad = (quad *)surface_out->geometry;
    surface_out->class = surface_quad;
    while (get_next_property(cursor, &property))
    {
        if (strcmp(property, "specular") == 0)
        {
            parse_color(cursor, &surface_out->specular_part);
        }
        else if (strcmp(property, "diffuse") == 0)
        {
            parse_color(cursor, &surface_out->diffuse_part);
        }
        else if (strcmp(property, "refraction_index") == 0)
        {
            parse_float(cursor, &surface_out->refraction_index);
        }
        else if (strcmp(property, "vertices") == 0)
        {
            parse_tuple_vector(cursor, cur_quad->vertices, 3);
        }
        else
        {
            fprintf(stderr, "Unknown quad property: %s\n", property);
        }
    }
    return 0;
}

int load_scene (FILE * file, scene * scene_out)
{
/*! Reads a input "file" stream, parsing it according to the file format
    defined above, and populate "scene_out" with the information parsed,
    dynamically allocating memory for the light source and surfaces array.
    The caller is responsible for calling "free" to release the memory
    allocated for the arrays.
*/
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

        if (strcmp(object_name, "camera") == 0)
        {
            parse_camera(&cursor, &scene_out->camera);
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
