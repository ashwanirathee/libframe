cmake_minimum_required(VERSION 3.10)
project(libframe C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_library(libframe
    src/transform.c
    src/neural.c
)

add_subdirectory(app)
