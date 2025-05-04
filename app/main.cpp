#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h" // loads vertex/fragment shaders
#include <Eigen/Dense>
#include "ray.h"
#include "pinhole.h"

const int width = 800;
const int height = 600;
Eigen::Vector3d cam_pos(0, 0, 0);
Eigen::Vector3d cam_forward(0, 0, -1);
Eigen::Vector3d cam_up(0, 1, 0);
double fov = 90.0;

PinholeCamera cam(cam_pos, cam_forward, cam_up, fov, double(width) / height);

unsigned int createTextureBuffer() {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    const double step = 0.1;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Eigen::Vector3d right = cam_forward.cross(cam_up).normalized();

        if (key == GLFW_KEY_W) cam_pos += step * cam_forward;
        if (key == GLFW_KEY_S) cam_pos -= step * cam_forward;
        if (key == GLFW_KEY_A) cam_pos -= step * right;
        if (key == GLFW_KEY_D) cam_pos += step * right;
        if (key == GLFW_KEY_Q) cam_pos += step * cam_up;
        if (key == GLFW_KEY_E) cam_pos -= step * cam_up;

        std::cout << "Camera position: " << cam_pos.transpose() << std::endl;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void generateRaytracedImage(unsigned char* buffer, PinholeCamera& cam) {
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            double u = double(i) / width;
            double v = double(j) / height;

            Ray ray = cam.getRay(u, v);
            Eigen::Vector3d color = ray_color(ray, v);

            int index = (j * width + i) * 3;
            buffer[index + 0] = static_cast<unsigned char>(255.99 * color.x());
            buffer[index + 1] = static_cast<unsigned char>(255.99 * color.y());
            buffer[index + 2] = static_cast<unsigned char>(255.99 * color.z());
        }
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(width, height, "Raytracer GPU Display", NULL, NULL);
    if (!window) {
        std::cerr << "GLFW window creation failed!" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set the key callback
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Shader setup
    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");

    // Quad vertices and texture coordinates
    float quadVertices[] = {
        // positions      // texCoords
        -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f,  1.0f,    1.0f, 1.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Texture setup
    unsigned int texture = createTextureBuffer();
    unsigned char* imageData = new unsigned char[width * height * 3];
    generateRaytracedImage(imageData, cam); // fills the buffer using ray_color()

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
    
        // Update camera
        cam = PinholeCamera(cam_pos, cam_forward, cam_up, fov, double(width) / height);
    
        // Generate image and upload to texture
        generateRaytracedImage(imageData, cam);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    
        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    delete[] imageData;
    glfwTerminate();
    return 0;
}
