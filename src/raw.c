#include <raw.h>

// Function to load the raw image data with dynamic width and height
int load_raw_image(const char *filename, uint16_t **image_data, size_t *image_size, size_t width, size_t height) {
    // Open the raw file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;  // Return an error code
    }

    // Calculate the size of the image data (width * height)
    *image_size = width * height;

    // Allocate memory for uint16_t data (2 bytes per pixel)
    *image_data = (uint16_t *)malloc(*image_size * sizeof(uint16_t));  // 2 bytes per pixel
    if (!*image_data) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;  // Return an error code
    }

    // Read the raw file data into the allocated memory
    size_t read_bytes = fread(*image_data, sizeof(uint16_t), *image_size, file);
    if (read_bytes != *image_size) {
        fprintf(stderr, "Unexpected file size. Read %zu bytes.\n", read_bytes);
        free(*image_data);
        fclose(file);
        return 1;  // Return an error code
    }

    // Close the file after reading
    fclose(file);

    return 0;  // Success
}
