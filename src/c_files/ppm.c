#include <ppm.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

void save_as_ppm(const char *filename, uint16_t *image_data, size_t width, size_t height, uint16_t min_val, uint16_t max_val) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Unable to open file for writing");
        return;
    }

    // Write PPM header
    fprintf(fp, "P6\n%zu %zu\n255\n", width, height);

    for (size_t i = 0; i < width * height; i++) {
        uint16_t val = image_data[i];

        // Normalize to 0-255
        uint8_t norm = (uint8_t)(((float)(val - min_val) / (max_val - min_val)) * 255.0f);

        // Write grayscale as RGB
        fputc(norm, fp);  // R
        fputc(norm, fp);  // G
        fputc(norm, fp);  // B
    }

    fclose(fp);
}