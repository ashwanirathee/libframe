add_executable(isp isp.cpp)

# Link libraries to the executable
target_link_libraries(isp PRIVATE frame)

target_include_directories(isp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(read_pi_temp read_pi_temp.cpp)

add_executable(read_webcams read_webcams.cpp)
