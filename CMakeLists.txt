cmake_minimum_required(VERSION 3.10)
project(frame C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 QUIET NO_MODULE)

if(Eigen3_FOUND)
    set(FRAME_EIGEN_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
else()
    find_path(FRAME_EIGEN_INCLUDE_DIR
        NAMES Eigen/Dense
        PATHS
            /usr/include/eigen3
            /usr/local/include/eigen3
            /opt/homebrew/include/eigen3
    )

    if(NOT FRAME_EIGEN_INCLUDE_DIR)
        message(FATAL_ERROR "Eigen headers not found. Install Eigen3 or set CMAKE_PREFIX_PATH to its location.")
    endif()

    set(FRAME_EIGEN_INCLUDE_DIRS ${FRAME_EIGEN_INCLUDE_DIR})
endif()

add_library(frame STATIC
	src/raw.c
	src/ppm.c
	src/dead_pixel_correction.c
	src/pinhole.cpp
)

target_include_directories(frame PUBLIC
	${FRAME_EIGEN_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(app)
