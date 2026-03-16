#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
void configure_camera(cv::VideoCapture &camera, int width, int height)
{
    camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
}

cv::Mat capture_frame(const std::string &device, int width, int height)
{
    cv::VideoCapture camera(device, cv::CAP_V4L2);
    if (!camera.isOpened())
    {
        throw std::runtime_error("failed to open camera " + device);
    }

    configure_camera(camera, width, height);

    cv::Mat frame;
    if (!camera.read(frame) || frame.empty())
    {
        throw std::runtime_error("failed to read frame from " + device);
    }

    return frame;
}

void save_frame(const std::string &path, const cv::Mat &frame)
{
    if (!cv::imwrite(path, frame))
    {
        throw std::runtime_error("failed to save frame to " + path);
    }
}
} // namespace

int main(int argc, char **argv)
{
    const std::string cam0 = argc > 1 ? argv[1] : "/dev/video0";
    const std::string cam1 = argc > 2 ? argv[2] : "/dev/video2";
    const int width = argc > 3 ? std::atoi(argv[3]) : 640;
    const int height = argc > 4 ? std::atoi(argv[4]) : 480;

    try
    {
        const cv::Mat frame0 = capture_frame(cam0, width, height);
        const cv::Mat frame1 = capture_frame(cam1, width, height);

        save_frame("cam0.jpg", frame0);
        save_frame("cam1.jpg", frame1);

        std::cout << cam0 << " -> cam0.jpg " << frame0.cols << "x" << frame0.rows << '\n';
        std::cout << cam1 << " -> cam1.jpg " << frame1.cols << "x" << frame1.rows << '\n';
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
