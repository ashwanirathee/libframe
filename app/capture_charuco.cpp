#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
struct Options
{
    std::string device = "/dev/video0";
    fs::path output_dir = "charuco_capture";
    std::string prefix = "charuco";
    std::string dictionary_name = "DICT_4X4_50";
    int squares_x = 5;
    int squares_y = 7;
    float square_length = 0.04f;
    float marker_length = 0.02f;
    int min_corners = 8;
    int width = 1280;
    int height = 720;
    double save_interval_sec = 1.5;
    int max_images = 0;
    bool headless = false;
};

int dictionary_id_from_name(const std::string &name)
{
    static const std::vector<std::pair<std::string, int>> kDictionaries = {
        {"DICT_4X4_50", cv::aruco::DICT_4X4_50},
        {"DICT_4X4_100", cv::aruco::DICT_4X4_100},
        {"DICT_4X4_250", cv::aruco::DICT_4X4_250},
        {"DICT_4X4_1000", cv::aruco::DICT_4X4_1000},
        {"DICT_5X5_50", cv::aruco::DICT_5X5_50},
        {"DICT_5X5_100", cv::aruco::DICT_5X5_100},
        {"DICT_5X5_250", cv::aruco::DICT_5X5_250},
        {"DICT_5X5_1000", cv::aruco::DICT_5X5_1000},
        {"DICT_6X6_50", cv::aruco::DICT_6X6_50},
        {"DICT_6X6_100", cv::aruco::DICT_6X6_100},
        {"DICT_6X6_250", cv::aruco::DICT_6X6_250},
        {"DICT_6X6_1000", cv::aruco::DICT_6X6_1000},
        {"DICT_7X7_50", cv::aruco::DICT_7X7_50},
        {"DICT_7X7_100", cv::aruco::DICT_7X7_100},
        {"DICT_7X7_250", cv::aruco::DICT_7X7_250},
        {"DICT_7X7_1000", cv::aruco::DICT_7X7_1000},
        {"DICT_ARUCO_ORIGINAL", cv::aruco::DICT_ARUCO_ORIGINAL},
    };

    for (const auto &entry : kDictionaries)
    {
        if (entry.first == name)
        {
            return entry.second;
        }
    }

    throw std::runtime_error("unknown dictionary: " + name);
}

Options parse_args(int argc, char **argv)
{
    Options options;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        auto require_value = [&](const std::string &flag) -> std::string {
            if (i + 1 >= argc)
            {
                throw std::runtime_error("missing value for " + flag);
            }
            ++i;
            return argv[i];
        };

        if (arg == "--device")
        {
            options.device = require_value(arg);
        }
        else if (arg == "--output-dir")
        {
            options.output_dir = require_value(arg);
        }
        else if (arg == "--prefix")
        {
            options.prefix = require_value(arg);
        }
        else if (arg == "--dictionary")
        {
            options.dictionary_name = require_value(arg);
        }
        else if (arg == "--squares-x")
        {
            options.squares_x = std::stoi(require_value(arg));
        }
        else if (arg == "--squares-y")
        {
            options.squares_y = std::stoi(require_value(arg));
        }
        else if (arg == "--square-length")
        {
            options.square_length = std::stof(require_value(arg));
        }
        else if (arg == "--marker-length")
        {
            options.marker_length = std::stof(require_value(arg));
        }
        else if (arg == "--min-corners")
        {
            options.min_corners = std::stoi(require_value(arg));
        }
        else if (arg == "--width")
        {
            options.width = std::stoi(require_value(arg));
        }
        else if (arg == "--height")
        {
            options.height = std::stoi(require_value(arg));
        }
        else if (arg == "--save-interval")
        {
            options.save_interval_sec = std::stod(require_value(arg));
        }
        else if (arg == "--max-images")
        {
            options.max_images = std::stoi(require_value(arg));
        }
        else if (arg == "--headless")
        {
            options.headless = true;
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout
                << "Usage: capture_charuco [--device /dev/video0] [--output-dir dir] [--prefix name]\n"
                << "                      [--dictionary DICT_4X4_50] [--squares-x 5] [--squares-y 7]\n"
                << "                      [--square-length 0.04] [--marker-length 0.02]\n"
                << "                      [--min-corners 8] [--width 1280] [--height 720]\n"
                << "                      [--save-interval 1.5] [--max-images 0] [--headless]\n";
            std::exit(0);
        }
        else
        {
            throw std::runtime_error("unknown argument: " + arg);
        }
    }

    return options;
}

std::string next_filename(const fs::path &output_dir, const std::string &prefix, int index)
{
    std::ostringstream oss;
    oss << prefix << "_" << std::setw(4) << std::setfill('0') << index << ".jpg";
    return (output_dir / oss.str()).string();
}
} // namespace

int main(int argc, char **argv)
{
    try
    {
        Options options = parse_args(argc, argv);
        if (std::getenv("DISPLAY") == nullptr && std::getenv("WAYLAND_DISPLAY") == nullptr)
        {
            options.headless = true;
        }
        fs::create_directories(options.output_dir);

        const auto dictionary = cv::aruco::getPredefinedDictionary(dictionary_id_from_name(options.dictionary_name));
        const auto board = cv::aruco::CharucoBoard::create(
            options.squares_x,
            options.squares_y,
            options.square_length,
            options.marker_length,
            dictionary);
        const auto detector_params = cv::aruco::DetectorParameters::create();

        cv::VideoCapture camera(options.device, cv::CAP_V4L2);
        if (!camera.isOpened())
        {
            throw std::runtime_error("failed to open camera " + options.device);
        }

        camera.set(cv::CAP_PROP_FRAME_WIDTH, options.width);
        camera.set(cv::CAP_PROP_FRAME_HEIGHT, options.height);

        int save_index = 0;
        for (const auto &entry : fs::directory_iterator(options.output_dir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".jpg")
            {
                ++save_index;
            }
        }

        if (options.headless)
        {
            std::cout << "Headless capture enabled\n";
            std::cout << "  auto-save when corners >= " << options.min_corners
                      << " every " << options.save_interval_sec << " seconds\n";
            if (options.max_images > 0)
            {
                std::cout << "  stop after " << options.max_images << " images\n";
            }
        }
        else
        {
            std::cout << "Capture controls:\n";
            std::cout << "  s = save frame when enough ChArUco corners are visible\n";
            std::cout << "  q = quit\n";
        }

        const auto save_interval = std::chrono::duration<double>(options.save_interval_sec);
        auto last_save_time = std::chrono::steady_clock::now() - save_interval;
        int frame_counter = 0;

        while (true)
        {
            cv::Mat frame;
            if (!camera.read(frame) || frame.empty())
            {
                throw std::runtime_error("failed to read frame from " + options.device);
            }

            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

            std::vector<std::vector<cv::Point2f>> marker_corners;
            std::vector<int> marker_ids;
            std::vector<std::vector<cv::Point2f>> rejected;

            cv::aruco::detectMarkers(gray, dictionary, marker_corners, marker_ids, detector_params, rejected);

            cv::Mat charuco_corners;
            cv::Mat charuco_ids;
            int charuco_count = 0;
            if (!marker_ids.empty())
            {
                charuco_count = cv::aruco::interpolateCornersCharuco(
                    marker_corners,
                    marker_ids,
                    gray,
                    board,
                    charuco_corners,
                    charuco_ids);
            }

            cv::Mat preview = frame.clone();
            if (!marker_ids.empty())
            {
                cv::aruco::drawDetectedMarkers(preview, marker_corners, marker_ids);
            }
            if (charuco_count >= 4 && !charuco_ids.empty())
            {
                cv::aruco::drawDetectedCornersCharuco(preview, charuco_corners, charuco_ids);
            }

            const bool ready_to_save = charuco_count >= options.min_corners;
            const auto now = std::chrono::steady_clock::now();

            if (options.headless)
            {
                if ((frame_counter % 30) == 0)
                {
                    std::cout << "frame " << frame_counter << ": detected " << charuco_count
                              << " corners\r" << std::flush;
                }

                if (ready_to_save && (now - last_save_time) >= save_interval)
                {
                    const std::string path = next_filename(options.output_dir, options.prefix, save_index);
                    if (!cv::imwrite(path, frame))
                    {
                        throw std::runtime_error("failed to save image to " + path);
                    }

                    std::cout << "\nsaved " << path << " with " << charuco_count << " corners\n";
                    ++save_index;
                    last_save_time = now;

                    if (options.max_images > 0 && save_index >= options.max_images)
                    {
                        break;
                    }
                }
            }
            else
            {
                const std::string status = ready_to_save
                    ? "ready"
                    : "need " + std::to_string(options.min_corners) + " corners";
                cv::putText(
                    preview,
                    "corners=" + std::to_string(charuco_count) + " " + status,
                    cv::Point(20, 30),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.8,
                    ready_to_save ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                    2);

                cv::imshow("capture_charuco", preview);
                const int key = cv::waitKey(10) & 0xFF;
                if (key == 'q' || key == 27)
                {
                    break;
                }
                if (key == 's')
                {
                    if (!ready_to_save)
                    {
                        std::cout << "not saved: only " << charuco_count << " corners detected\n";
                    }
                    else
                    {
                        const std::string path = next_filename(options.output_dir, options.prefix, save_index);
                        if (!cv::imwrite(path, frame))
                        {
                            throw std::runtime_error("failed to save image to " + path);
                        }

                        std::cout << "saved " << path << " with " << charuco_count << " corners\n";
                        ++save_index;
                    }
                }
            }

            ++frame_counter;
        }

        if (!options.headless)
        {
            cv::destroyAllWindows();
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
