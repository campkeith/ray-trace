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
    aperture * aperture = &scene->aperture;

    float h_angle = scene->aperture.view_angle;
    float v_angle = h_angle * (float)aperture->pixels_high / (float)aperture->pixels_wide;

    for (y = 0; y < aperture->pixels_high; y++)
    {
        phi = v_angle * ((float)y / (float)(aperture->pixels_high - 1) - 0.5f);
        for (x = 0; x < aperture->pixels_wide; x++)
        {
            theta = h_angle * ((float)x / (float)(aperture->pixels_wide - 1) - 0.5f);

            ray = vector_rotate(vector_theta_phi(theta, phi), aperture->theta, aperture->phi);
            pixel_color = cast_ray(scene, aperture->position, ray, depth);

            image_out[(aperture->pixels_high - y - 1) * aperture->pixels_wide + x] = pixel_color;
        }
    }
}

int main ()
{
    scene cur_scene;
    color * image;
    if (load_scene(stdin, &cur_scene))
    {
        perror("Scene load");
        return -1;
    }
    image = malloc(sizeof(color) * cur_scene.aperture.pixels_wide * cur_scene.aperture.pixels_high);

    render(&cur_scene, image);

    if (save_image(image, cur_scene.aperture.pixels_wide, cur_scene.aperture.pixels_high, stdout))
    {
        perror("Image save");
        return -1;
    }
    else
    {
        return 0;
    }
}
