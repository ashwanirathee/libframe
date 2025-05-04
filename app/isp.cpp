#include <iostream>
#include <stdlib.h>
#include <raw.h>

int main()
{
    uint16_t *image_data = NULL;
    size_t image_size = 0;

    // Set the image dimensions (for example, 1920x1080)
    size_t width = 1920;
    size_t height = 1080;

    // Call the function to load the raw image
    int result = load_raw_image("test.RAW", &image_data, &image_size, width, height);
    if (result != 0) {
        // If loading fails, exit the program with an error code
        return 1;
    }

    // Example: Print top-left 5x5 pixel values
    std::cout << "Top-left 5x5 pixel values:\n";
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            std::cout << image_data[y * width + x] << " ";
        }
        std::cout << std::endl;
    }

    // Free the allocated memory
    free(image_data);
    return 0;
}
