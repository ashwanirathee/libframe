#include <renderer.h>
#include <Eigen/Dense>
#include <iostream>


unsigned int createTextureBuffer(const int width, const int height)
{
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

void generateRaytracedImage(unsigned char *buffer, PinholeCamera &cam, const int height, const int width)
{
    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            double u = double(i) / width;
            double v = double(j) / height;

            Ray ray = cam.getRay(u, v);
            Eigen::Vector3d color = ray_color(ray, v);

            int index = (j * width + i) * 3;
            buffer[index + 0] = static_cast<unsigned char>(255.99 * color.x());
            buffer[index + 1] = static_cast<unsigned char>(255.99 * color.y());
            buffer[index + 2] = static_cast<unsigned char>(255.99 * color.z());

            if (i == 0 && j == 0) {
                std::cout << "Color at (0,0): " 
                          << (int)buffer[index + 0] << ", " 
                          << (int)buffer[index + 1] << ", " 
                          << (int)buffer[index + 2] << std::endl;
            }
        }
    }
}


Renderer::Renderer(const int width, const int height)
{
    this->width = width;
    this->height = height;

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->window = glfwCreateWindow(this->width, this->height, "Raytracer GPU Display", NULL, NULL);
    if (!window)
    {
        std::cerr << "GLFW window creation failed!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    shader = new Shader("vertex_shader.glsl", "fragment_shader.glsl");

    // Initialize imageData here
    imageData = new unsigned char[width * height * 3];
}

Renderer::~Renderer()
{
    glfwDestroyWindow(window);
    delete[] imageData;
    glfwTerminate();
    delete shader;  // Free the shader object
}

void Renderer::render(PinholeCamera &cam)
{
    // Set up OpenGL state
    // glEnable(GL_DEPTH_TEST);
    // glViewport(0, 0, width, height);

    // No need to instantiate shader again, just use the one created in the constructor
    // shader->use();  // Use the existing shader

    // Quad vertices and texture coordinates
    float quadVertices[] = {
        // positions      // texCoords
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    // Texture setup
    unsigned int texture = createTextureBuffer(width, height);

    // Generate raytraced image once, outside the loop
    generateRaytracedImage(imageData, cam, width, height); // fill the buffer once

    // Upload the image data to the texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        shader->use();
        // Bind the texture and render the quad
        glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
    }
}