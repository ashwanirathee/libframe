cmake_minimum_required(VERSION 3.10)
project(frame C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

add_library(frame STATIC
	src/raw.c
	src/ppm.c
	src/dead_pixel_correction.c
	src/pinhole.cpp
)

target_include_directories(frame PUBLIC
	/opt/homebrew/include/eigen3
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(app)
