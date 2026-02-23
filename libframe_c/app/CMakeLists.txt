add_executable(isp isp.cpp)

# Link libraries to the executable
target_link_libraries(isp PRIVATE libframe_cpp)

add_executable(raytrace raytrace.cpp)

# Link libraries to the executable
target_link_libraries(raytrace PRIVATE libframe_cpp glad glfw OpenGL::GL)
target_include_directories(libframe_cpp PRIVATE
    external/glad/include
)