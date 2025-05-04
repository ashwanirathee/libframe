#define GL_SILENCE_DEPRECATION
#include <stdio.h>
#include "transform.h"
#include "neural.h"
#include "ray.h"
#include "pinhole.h"
#include <iostream>
#include <fstream>
#include <GLFW/glfw3.h>

using namespace Eigen;
using namespace std;

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    // Create a GLFW window
    int width = 800, height = 600;
    GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL Raytracing", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Test transform.c
    int transform_result = transform_hello_world();
    printf("Transform result: %d\n", transform_result);

    // Test neural.c
    int nn_result = nn_hello_world();
    printf("NN result: %d\n", nn_result);
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / (16.0 / 9.0));
    const double aspect_ratio = double(image_width) / image_height;

    PinholeCamera cam(
        Eigen::Vector3d(0, 0, 0),
        Eigen::Vector3d(0, 0, -1),
        Eigen::Vector3d(0, 1, 0),
        90.0,
        16.0 / 9.0);

    Ray r = cam.getRay(0.5, 0.5);
    std::cout << "Ray origin: " << r.origin.transpose() << std::endl;
    std::cout << "Ray direction: " << r.direction.transpose() << std::endl;

    std::ofstream out("image.ppm");
    out << "P3\n"
        << image_width << " " << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j)
    {
        for (int i = 0; i < image_width; ++i)
        {
            double u = double(i) / (image_width - 1);
            double v = double(j) / (image_height - 1);
            Ray r = cam.getRay(u, v);
            Eigen::Vector3d color = rayColor(r);

            int ir = static_cast<int>(255.999 * color.x());
            int ig = static_cast<int>(255.999 * color.y());
            int ib = static_cast<int>(255.999 * color.z());

            out << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    out.close();
    std::cout << "Rendered image written to image.ppm\n";

    // OpenGL rendering loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // Raytracing: Generate rays for each pixel and render the result
        for (int j = height - 1; j >= 0; --j)
        {
            for (int i = 0; i < width; ++i)
            {
                double u = double(i) / double(width);
                double v = double(j) / double(height);

                // Get the ray from the camera
                Ray r = cam.getRay(u, v);

                // Get the color for this ray
                Eigen::Vector3d color = rayColor(r);

                // Set the pixel color (Note: OpenGL uses RGBA values, so we convert Eigen::Vector3d to float[4])
                glBegin(GL_POINTS);
                glColor3f(color.x(), color.y(), color.z()); // Use the RGB color from raytracing
                glVertex2i(i, j);
                glEnd();
            }
        }

        // Swap buffers and process events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
