#include "output_file.h"

#include <stdio.h>

unsigned char convert_to_8_bit (float float_val)
{
    int value = (float)0x100 * float_val;
    if (value < 0)
    {
        value = 0;
    }
    if (value > 0xff)
    {
        value = 0xff;
    }
    return value;
}

int save_image (color image[], int width, int height, FILE * file)
{
    const int max_value = 0xff;
    unsigned char output[3];
    color pixel_color;
    int x, y;

    fprintf(file, "P6\n");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "%d\n", max_value);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel_color = image[y * width + x];
            output[0] = convert_to_8_bit(pixel_color.r);
            output[1] = convert_to_8_bit(pixel_color.g);
            output[2] = convert_to_8_bit(pixel_color.b);
            
            fwrite(output, 1, 3, file);
        }
    }
    return 0;
}
