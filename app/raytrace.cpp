#include <iostream>
#include <Eigen/Dense>
#include "pinhole.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer.h"

const int width = 800;
const int height = 600;
Eigen::Vector3d cam_pos(0, 0, 0);
Eigen::Vector3d cam_forward(0, 0, -1);
Eigen::Vector3d cam_up(0, 1, 0);
double fov = 90.0;
bool cameraMoved = true; // trigger initial render

PinholeCamera cam(cam_pos, cam_forward, cam_up, fov, double(width) / height);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    const double step = 0.1;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        Eigen::Vector3d right = cam_forward.cross(cam_up).normalized();

        if (key == GLFW_KEY_W)
            cam_pos += step * cam_forward;
        if (key == GLFW_KEY_S)
            cam_pos -= step * cam_forward;
        if (key == GLFW_KEY_A)
            cam_pos -= step * right;
        if (key == GLFW_KEY_D)
            cam_pos += step * right;
        if (key == GLFW_KEY_Q)
            cam_pos += step * cam_up;
        if (key == GLFW_KEY_E)
            cam_pos -= step * cam_up;
        cameraMoved = true; // trigger raytracing
        std::cout << "Camera position: " << cam_pos.transpose() << std::endl;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main()
{
    std::cout << "Hello, World!" << std::endl;
    Renderer renderer(width, height);
    renderer.render(cam);
    glfwSetKeyCallback(renderer.window, key_callback);
    return 0;
}