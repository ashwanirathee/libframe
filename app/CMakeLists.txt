add_executable(app_main main.cpp shader.cpp)

# Link libraries to the executable
target_link_libraries(app_main PRIVATE libframe_cpp glad glfw OpenGL::GL)
target_include_directories(libframe_cpp PRIVATE
    external/glad/include
)