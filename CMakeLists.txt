cmake_minimum_required(VERSION 3.10)
project(libframe C CXX)  # Note: Both C and C++ languages

# Set C and C++ standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# Find Eigen (Homebrew installs it here on macOS ARM)
include_directories(/opt/homebrew/include/eigen3)

# Find OpenGL and GLFW
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)

# Manually find GLFW (if not using package manager)
find_package(GLFW3 REQUIRED PATHS /opt/homebrew/opt/glfw/lib/cmake/glfw3)

# GLAD
add_library(glad
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC external/glad/include)

# Include directories
include_directories(include)

# Add C sources (C files in src/c_files)
add_library(libframe_c STATIC
    src/c_files/transform.c
    src/c_files/neural.c
    src/c_files/raw.c
    src/c_files/ppm.c
    src/c_files/dead_pixel_correction.c
)

# Add C++ sources (C++ files in src/cpp_files)
add_library(libframe_cpp STATIC
    src/cpp_files/pinhole.cpp
    src/cpp_files/ray.cpp
    src/cpp_files/renderer.cpp
    src/cpp_files/shader.cpp
)

# Link libraries (C++ code will link to C code, if necessary)
target_link_libraries(libframe_cpp PRIVATE libframe_c glad glfw OpenGL::GL)

target_include_directories(libframe_cpp PRIVATE
    external/glad/include
)

# Add subdirectory for app (where your main.cpp is)
add_subdirectory(app)
