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

    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / (16.0 / 9.0));
    const double aspect_ratio = double(image_width) / image_height;

    PinholeCamera cam(
        Eigen::Vector3d(0, 0, 0),
        Eigen::Vector3d(0, 0, -1),
        Eigen::Vector3d(0, 1, 0),
        90.0,
        16.0 / 9.0);

    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPointSize(1.0f);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glBegin(GL_POINTS);
        // Raytracing: Generate rays for each pixel and render the result
        for (int j = height - 1; j >= 0; --j)
        {
            for (int i = 0; i < width; ++i)
            {
                double u = double(i) / double(width);
                double v = double(j) / double(height);

                Ray r = cam.getRay(u, v);
                Eigen::Vector3d color = ray_color(r, v);

                glColor3f(color.x(), color.y(), color.z()); // Use the RGB color from raytracing
                glVertex2i(i, j);
            }
        }
        glEnd();
        glfwSwapBuffers(window); // Swap buffers and process events
        glfwPollEvents();
    }

    // Clean up and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
