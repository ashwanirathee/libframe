add_executable(isp isp.cpp)

# Link libraries to the executable
target_link_libraries(isp PRIVATE frame)

target_include_directories(isp PUBLIC
    /opt/homebrew/include/eigen3
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)