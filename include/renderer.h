#ifndef RENDERER_H
#define RENDERER_H

#define GL_SILENCE_DEPRECATION
#include "ray.h"
#include "pinhole.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h" // loads vertex/fragment shaders

class Renderer
{
public:
    Renderer(const int width, const int height);
    ~Renderer();
    void render(PinholeCamera &cam);
    GLFWwindow *window;
    int width;
    int height;
    unsigned char *imageData;
    Shader* shader;
};

unsigned int createTextureBuffer(const int width, const int height);
void generateRaytracedImage(unsigned char *buffer, PinholeCamera &cam, const int height, const int width);

#endif