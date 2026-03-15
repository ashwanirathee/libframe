#include <dead_pixel_correction.h>
#include <stdint.h>
#include <limits.h>

void dead_pixel_correction(uint16_t *image_data, size_t width, size_t height, float threshold)
{
    printf("Width*Height: %zu\n", width * height);
    int min_value = INT_MAX;
    int max_value = INT_MIN;
    int max_pixel_loc = 0;
    for (size_t x = 0; x < width - 4; x++)
    {
        for (size_t y = 0; y < height - 4; y++)
        {
            uint16_t p0 = image_data[(y + 2) * width + (x + 2)];
            uint16_t p1 = image_data[(y)*width + (x)];
            uint16_t p2 = image_data[(y)*width + (x + 2)];
            uint16_t p3 = image_data[(y)*width + (x + 4)];
            uint16_t p4 = image_data[(y + 2) * width + (x)];
            uint16_t p5 = image_data[(y + 2) * width + (x + 4)];
            uint16_t p6 = image_data[(y + 4) * width + (x)];
            uint16_t p7 = image_data[(y + 4) * width + (x + 2)];
            uint16_t p8 = image_data[(y + 4) * width + (x + 4)];

            if (abs(p1 - p0) > threshold && abs(p2 - p0) > threshold && abs(p3 - p0) > threshold &&
                abs(p4 - p0) > threshold && abs(p5 - p0) > threshold && abs(p6 - p0) > threshold &&
                abs(p7 - p0) > threshold && abs(p8 - p0) > threshold)
            {
                // printf("Dead pixel detected at (%zu, %zu): %u\n", x + 2, y + 2, p0);
                // Replace the dead pixel with the average of its neighbors
                image_data[(y + 2) * width + (x + 2)] = (p2 + p4 + p5 + p7) / 4;
            }

            // printf("Pixel value at (%zu, %zu): %u\n", x, y, pixel_value);
            if (p0 < min_value)
            {
                min_value = p0;
            }
            if (p0 > max_value)
            {
                max_value = p0;
            }

            if (max_pixel_loc < (y + 4) * width + (x + 4))
            {
                max_pixel_loc = (y + 4) * width + (x + 4);
            }
        }
    }
    printf("Min value: %d, Max value: %d\n", min_value, max_value);
    printf("Max pixel location: %d\n", max_pixel_loc);
}