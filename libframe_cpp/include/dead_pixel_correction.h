#ifndef DEAD_PIXEL_CORRECTION_H
#define DEAD_PIXEL_CORRECTION_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void dead_pixel_correction(uint16_t *image_data, size_t width, size_t height, float threshold);

#ifdef __cplusplus
}
#endif

#endif // DEAD_PIXEL_CORRECTION_H