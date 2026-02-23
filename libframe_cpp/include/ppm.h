#ifndef PPM_H
#define PPM_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void save_as_ppm(const char *filename, uint16_t *image_data, size_t width, size_t height, uint16_t min_val, uint16_t max_val);

#ifdef __cplusplus
}
#endif

#endif