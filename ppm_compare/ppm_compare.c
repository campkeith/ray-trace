#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} color;

typedef struct
{
    int width;
    int height;
    color * pixels;
} image;

void read_ppm (char * image_file, image * image_out)
{
    FILE * stream = fopen(image_file, "r");
    if (stream == NULL)
    {
        fprintf(stderr, "Failed to open \"%s\": %s\n", image_file, strerror(errno));
        exit(-1);
    }

    int width, height, max_value;
    char format[3];
    int args_parsed = fscanf(stream, "%2s %d %d %d", format, &width, &height, &max_value);
    if (args_parsed != 4 || strcmp(format, "P6") != 0)
    {
        fprintf(stderr, "\"%s\": not a P6 ppm file\n", image_file);
        exit(-1);
    }
    if (max_value != 255)
    {
        fprintf(stderr, "\"%s\": only max pixel value of 255 is supported\n", image_file);
        exit(-1);
    }

    color * image_buffer = calloc(width * height, sizeof(color));
    if (image_buffer == NULL)
    {
        perror("calloc");
        exit(-1);
    }

    char byte;
    fread(&byte, 1, 1, stream);
    int pixels_read = fread(image_buffer, sizeof(color), width * height, stream);
    int more_bytes = fread(&byte, 1, 1, stream);

    if (pixels_read != width * height || more_bytes)
    {
        fprintf(stderr, "\"%s\" appears to be corrupted\n", image_file);
        exit(-1);
    }
    fclose(stream);

    image_out->width = width;
    image_out->height = height;
    image_out->pixels = image_buffer;
}

int main (int argc, char * argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <image1.ppm> <image2.ppm>\n", argv[0]);
        return 1;
    }
    
    image image1, image2;
    read_ppm(argv[1], &image1);
    read_ppm(argv[2], &image2);

    if (image1.width != image2.width || image2.height != image2.height)
    {
        fprintf(stderr, "Image size mismatch\n");
        return 1;
    }
    int width = image1.width;
    int height = image1.height;
    int64_t difference = 0;
    for (int index = 0; index < width * height; index++)
    {
        difference += abs((int)image1.pixels[index].r - (int)image2.pixels[index].r);
        difference += abs((int)image1.pixels[index].g - (int)image2.pixels[index].g);
        difference += abs((int)image1.pixels[index].b - (int)image2.pixels[index].b);
    }

    int64_t max_difference = 255LL * (int64_t)width * (int64_t)height * 3LL;
    float relative_diff = (float)difference / (float)max_difference;

    int score = 0;
    float bar = 0.5f;
    while (relative_diff < bar && score != 10)
    {
        score++;
        bar *= 0.5f;
    }

    printf("Raw match result: %f%%\n", (1.0f - relative_diff) * 100.0f);
    printf("Match score: %d/10\n", score);
    return 0;
}
