#ifndef TRANSFORM_H
#define TRANSFORM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>

int load_raw_image(const char *filename, uint16_t **image_data, size_t *image_size, size_t width, size_t height);

#ifdef __cplusplus
}
#endif

#endif