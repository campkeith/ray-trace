#include "input_file.h"
#include "output_file.h"
#include "ray_trace.h"
#include "surface.h"
#include "vector.h"
#include "scene.h"
#include "color.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

const int depth = 8;

void handle_args (int argc, char * argv[], FILE ** input_stream, FILE ** output_stream)
{
    char * scene_filename;
    char * image_filename;
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_scene_file> <output_ppm_file>\n", argv[0]);
        exit(1);
    }
    
    scene_filename = argv[1];
    image_filename = argv[2];
    
    *input_stream = fopen(scene_filename, "r");
    if (*input_stream == NULL)
    {
        fprintf(stderr, "Unable to open input scene file %s: %s\n",
                        scene_filename, strerror(errno));
        exit(1);
    }
    
    *output_stream = fopen(image_filename, "w");
    if (*output_stream == NULL)
    {
        fprintf(stderr, "Unable to open output image file %s: %s\n",
                        image_filename, strerror(errno));
        exit(1);
    }
}

void render (scene * scene, color image_out[])
{
    vector ray;
    int x, y;
    float theta, phi;
    color pixel_color;
    resolution * res = &scene->camera.resolution;
    direction * dir = &scene->camera.direction;

    float h_angle = scene->camera.view_angle;
    float v_angle = h_angle * (float)res->height / (float)res->width;

    for (y = 0; y < res->height; y++)
    {
        phi = v_angle * ((float)y / (float)(res->height - 1) - 0.5f);
        for (x = 0; x < res->width; x++)
        {
            theta = h_angle * -((float)x / (float)(res->width - 1) - 0.5f);

            ray = vector_rotate(vector_theta_phi(theta, phi), dir->theta, dir->phi);
            pixel_color = cast_ray(scene, scene->camera.position, ray, depth);

            image_out[(res->height - y - 1) * res->width + x] = pixel_color;
        }
    }
}

int main (int argc, char * argv[])
{
    FILE * scene_file;
    FILE * image_file;
    scene cur_scene = {};
    color * image;
    resolution * res = &cur_scene.camera.resolution;
    
    handle_args (argc, argv, &scene_file, &image_file);

    if (load_scene(scene_file, &cur_scene))
    {
        perror("Scene load");
        return -1;
    }
    fclose(scene_file);
    image = malloc(sizeof(color) * res->width * res->height);

    render(&cur_scene, image);
    free(cur_scene.light_sources);
    free(cur_scene.surfaces);

    if (save_image(image, res->width, res->height, image_file))
    {
        perror("Image save");
        return -1;
    }
    fclose(image_file);
    free(image);
    
    return 0;
}
