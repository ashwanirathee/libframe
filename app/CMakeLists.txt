add_executable(isp isp.cpp)

# Link libraries to the executable
target_link_libraries(isp PRIVATE frame)

target_include_directories(isp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(read_pi_temp read_pi_temp.cpp)
add_executable(read_webcams read_webcams.cpp)

find_package(OpenCV QUIET COMPONENTS aruco calib3d core highgui imgcodecs imgproc videoio)

if(OpenCV_FOUND)
    add_executable(read_webcams_opencv read_webcams_opencv.cpp)
    target_link_libraries(read_webcams_opencv PRIVATE ${OpenCV_LIBS})
    add_executable(capture_charuco capture_charuco.cpp)
    target_link_libraries(capture_charuco PRIVATE ${OpenCV_LIBS})
    add_executable(calibrate_camera calibrate_camera.cpp)
    target_link_libraries(calibrate_camera PRIVATE ${OpenCV_LIBS})
else()
    message(STATUS "OpenCV not found; skipping read_webcams_opencv, capture_charuco, and calibrate_camera targets")
endif()
