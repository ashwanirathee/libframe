add_executable(app_main main.cpp)

# Link libraries to the executable
target_link_libraries(app_main PRIVATE libframe_cpp glfw OpenGL::GL)
