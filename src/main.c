#include "input_file.h"
#include "output_file.h"
#include "ray_trace.h"
#include "surface.h"
#include "vector.h"
#include "scene.h"
#include "color.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const int depth = 8;

void render (scene * scene, color image_out[])
{
    vector ray;
    int x, y;
    float theta, phi;
    color pixel_color;
    resolution * res = &scene->aperture.resolution;
    direction * dir = &scene->aperture.direction;

    float h_angle = scene->aperture.view_angle;
    float v_angle = h_angle * (float)res->height / (float)res->width;

    for (y = 0; y < res->height; y++)
    {
        phi = v_angle * ((float)y / (float)(res->height - 1) - 0.5f);
        for (x = 0; x < res->width; x++)
        {
            theta = h_angle * -((float)x / (float)(res->width - 1) - 0.5f);

            ray = vector_rotate(vector_theta_phi(theta, phi), dir->theta, dir->phi);
            pixel_color = cast_ray(scene, scene->aperture.position, ray, depth);

            image_out[(res->height - y - 1) * res->width + x] = pixel_color;
        }
    }
}

int main ()
{
    scene cur_scene;
    color * image;
    resolution * res = &cur_scene.aperture.resolution;
    if (load_scene(stdin, &cur_scene))
    {
        perror("Scene load");
        return -1;
    }
    image = malloc(sizeof(color) * res->width * res->height);

    render(&cur_scene, image);

    if (save_image(image, res->width, res->height, stdout))
    {
        perror("Image save");
        return -1;
    }
    else
    {
        return 0;
    }
}
