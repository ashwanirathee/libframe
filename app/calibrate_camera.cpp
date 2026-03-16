#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
struct Options
{
    std::vector<std::string> inputs;
    std::string output = "calibration_charuco.json";
    std::string dictionary_name = "DICT_4X4_50";
    int squares_x = 0;
    int squares_y = 0;
    float square_length = 0.0f;
    float marker_length = 0.0f;
    int min_frames = 8;
    double max_frame_error = 3.0;
    int max_rejection_passes = 3;
    bool preview = false;
};

struct FrameObservation
{
    fs::path image_path;
    cv::Mat charuco_corners;
    cv::Mat charuco_ids;
};

bool has_image_extension(const fs::path &path)
{
    const std::string ext = path.extension().string();
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp";
}

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

        if (arg == "--output")
        {
            options.output = require_value(arg);
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
        else if (arg == "--min-frames")
        {
            options.min_frames = std::stoi(require_value(arg));
        }
        else if (arg == "--max-frame-error")
        {
            options.max_frame_error = std::stod(require_value(arg));
        }
        else if (arg == "--max-rejection-passes")
        {
            options.max_rejection_passes = std::stoi(require_value(arg));
        }
        else if (arg == "--preview")
        {
            options.preview = true;
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout
                << "Usage: calibrate_camera [images|dirs ...] --squares-x N --squares-y N "
                << "--square-length F --marker-length F [--dictionary DICT_4X4_50] "
                << "[--output file.json] [--min-frames N] [--max-frame-error px] "
                << "[--max-rejection-passes N] [--preview]\n";
            std::exit(0);
        }
        else
        {
            options.inputs.push_back(arg);
        }
    }

    if (options.inputs.empty())
    {
        throw std::runtime_error("provide at least one image path or directory");
    }
    if (options.squares_x <= 0 || options.squares_y <= 0)
    {
        throw std::runtime_error("squares-x and squares-y must be positive");
    }
    if (options.square_length <= 0.0f || options.marker_length <= 0.0f)
    {
        throw std::runtime_error("square-length and marker-length must be positive");
    }

    return options;
}

std::vector<fs::path> expand_inputs(const std::vector<std::string> &inputs)
{
    std::vector<fs::path> paths;
    std::set<fs::path> seen;

    for (const std::string &input : inputs)
    {
        const fs::path path(input);
        if (fs::is_directory(path))
        {
            for (const auto &entry : fs::directory_iterator(path))
            {
                if (entry.is_regular_file() && has_image_extension(entry.path()))
                {
                    const fs::path resolved = fs::absolute(entry.path());
                    if (seen.insert(resolved).second)
                    {
                        paths.push_back(resolved);
                    }
                }
            }
            continue;
        }

        if (fs::is_regular_file(path))
        {
            const fs::path resolved = fs::absolute(path);
            if (seen.insert(resolved).second)
            {
                paths.push_back(resolved);
            }
        }
    }

    std::sort(paths.begin(), paths.end());
    return paths;
}

void save_calibration_json(const fs::path &output_path,
                           const cv::Size &image_size,
                           double rms,
                           const cv::Mat &camera_matrix,
                           const cv::Mat &dist_coeffs)
{
    fs::create_directories(output_path.parent_path());

    std::ofstream out(output_path);
    if (!out)
    {
        throw std::runtime_error("failed to open output file: " + output_path.string());
    }

    out << "{\n";
    out << "  \"image_width\": " << image_size.width << ",\n";
    out << "  \"image_height\": " << image_size.height << ",\n";
    out << "  \"rms_reprojection_error\": " << rms << ",\n";
    out << "  \"camera_matrix\": [\n";
    for (int r = 0; r < camera_matrix.rows; ++r)
    {
        out << "    [";
        for (int c = 0; c < camera_matrix.cols; ++c)
        {
            out << camera_matrix.at<double>(r, c);
            if (c + 1 < camera_matrix.cols)
            {
                out << ", ";
            }
        }
        out << "]";
        out << (r + 1 < camera_matrix.rows ? ",\n" : "\n");
    }
    out << "  ],\n";
    out << "  \"dist_coefficients\": [";
    for (int i = 0; i < dist_coeffs.total(); ++i)
    {
        out << dist_coeffs.at<double>(i);
        if (i + 1 < dist_coeffs.total())
        {
            out << ", ";
        }
    }
    out << "]\n";
    out << "}\n";
}

std::vector<cv::Point3f> charuco_object_points(const cv::Ptr<cv::aruco::CharucoBoard> &board,
                                               const cv::Mat &charuco_ids)
{
    std::vector<cv::Point3f> object_points;
    object_points.reserve(charuco_ids.total());

    const auto &board_corners = board->chessboardCorners;
    for (int i = 0; i < charuco_ids.rows; ++i)
    {
        const int id = charuco_ids.at<int>(i, 0);
        if (id < 0 || id >= static_cast<int>(board_corners.size()))
        {
            throw std::runtime_error("detected ChArUco corner id out of range");
        }
        object_points.push_back(board_corners[static_cast<size_t>(id)]);
    }

    return object_points;
}

double compute_frame_error(const cv::Ptr<cv::aruco::CharucoBoard> &board,
                           const FrameObservation &observation,
                           const cv::Mat &camera_matrix,
                           const cv::Mat &dist_coeffs,
                           const cv::Mat &rvec,
                           const cv::Mat &tvec)
{
    const std::vector<cv::Point3f> object_points = charuco_object_points(board, observation.charuco_ids);

    std::vector<cv::Point2f> projected_points;
    cv::projectPoints(object_points, rvec, tvec, camera_matrix, dist_coeffs, projected_points);

    double squared_error = 0.0;
    for (int i = 0; i < observation.charuco_corners.rows; ++i)
    {
        const cv::Point2f observed = observation.charuco_corners.at<cv::Point2f>(i, 0);
        const cv::Point2f projected = projected_points[static_cast<size_t>(i)];
        const cv::Point2f delta = observed - projected;
        squared_error += delta.dot(delta);
    }

    return std::sqrt(squared_error / static_cast<double>(observation.charuco_corners.rows));
}

void print_frame_errors(const std::vector<FrameObservation> &observations,
                        const std::vector<double> &frame_errors)
{
    std::cout << "per-image reprojection error:\n";
    for (size_t i = 0; i < observations.size(); ++i)
    {
        std::cout << "  " << observations[i].image_path.filename().string()
                  << ": " << frame_errors[i] << " px\n";
    }
}
} // namespace

int main(int argc, char **argv)
{
    try
    {
        const Options options = parse_args(argc, argv);
        const auto image_paths = expand_inputs(options.inputs);
        if (image_paths.empty())
        {
            throw std::runtime_error("no input images found");
        }

        const auto dictionary = cv::aruco::getPredefinedDictionary(dictionary_id_from_name(options.dictionary_name));
        const auto board = cv::aruco::CharucoBoard::create(
            options.squares_x,
            options.squares_y,
            options.square_length,
            options.marker_length,
            dictionary);
        const auto detector_params = cv::aruco::DetectorParameters::create();

        std::vector<FrameObservation> observations;
        cv::Size image_size;
        int used_images = 0;

        for (const auto &image_path : image_paths)
        {
            cv::Mat image = cv::imread(image_path.string(), cv::IMREAD_COLOR);
            if (image.empty())
            {
                std::cerr << "skipping unreadable image: " << image_path << '\n';
                continue;
            }

            cv::Mat gray;
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
            image_size = gray.size();

            std::vector<std::vector<cv::Point2f>> corners;
            std::vector<int> ids;
            std::vector<std::vector<cv::Point2f>> rejected;

            cv::aruco::detectMarkers(gray, dictionary, corners, ids, detector_params, rejected);
            if (ids.empty())
            {
                std::cout << "skipping " << image_path.filename().string() << ": no markers detected\n";
                continue;
            }

            cv::aruco::refineDetectedMarkers(gray, board, corners, ids, rejected);

            cv::Mat charuco_corners;
            cv::Mat charuco_ids;
            const int count = cv::aruco::interpolateCornersCharuco(
                corners,
                ids,
                gray,
                board,
                charuco_corners,
                charuco_ids);

            if (count < 4 || charuco_ids.empty())
            {
                std::cout << "skipping " << image_path.filename().string() << ": no usable ChArUco detection\n";
                continue;
            }

            observations.push_back({image_path, charuco_corners, charuco_ids});
            ++used_images;
            std::cout << "using " << image_path.filename().string() << ": "
                      << charuco_ids.total() << " corners\n";

            if (options.preview)
            {
                cv::Mat preview = image.clone();
                cv::aruco::drawDetectedCornersCharuco(preview, charuco_corners, charuco_ids);
                cv::imshow("charuco", preview);
                if (cv::waitKey(250) == 27)
                {
                    break;
                }
            }
        }

        if (options.preview)
        {
            cv::destroyAllWindows();
        }

        if (used_images < options.min_frames)
        {
            throw std::runtime_error(
                "not enough usable images for calibration: got " + std::to_string(used_images) +
                ", need at least " + std::to_string(options.min_frames));
        }

        cv::Mat camera_matrix;
        cv::Mat dist_coeffs;
        std::vector<cv::Mat> rvecs;
        std::vector<cv::Mat> tvecs;
        double rms = 0.0;

        for (int pass = 0; pass <= options.max_rejection_passes; ++pass)
        {
            std::vector<cv::Mat> all_charuco_corners;
            std::vector<cv::Mat> all_charuco_ids;
            all_charuco_corners.reserve(observations.size());
            all_charuco_ids.reserve(observations.size());
            for (const auto &observation : observations)
            {
                all_charuco_corners.push_back(observation.charuco_corners);
                all_charuco_ids.push_back(observation.charuco_ids);
            }

            rvecs.clear();
            tvecs.clear();
            rms = cv::aruco::calibrateCameraCharuco(
                all_charuco_corners,
                all_charuco_ids,
                board,
                image_size,
                camera_matrix,
                dist_coeffs,
                rvecs,
                tvecs);

            std::vector<double> frame_errors;
            frame_errors.reserve(observations.size());
            for (size_t i = 0; i < observations.size(); ++i)
            {
                frame_errors.push_back(compute_frame_error(
                    board, observations[i], camera_matrix, dist_coeffs, rvecs[i], tvecs[i]));
            }

            std::cout << "rejection pass " << pass << ": rms=" << rms
                      << " using " << observations.size() << " frames\n";
            print_frame_errors(observations, frame_errors);

            if (pass == options.max_rejection_passes)
            {
                break;
            }

            const auto worst_it = std::max_element(frame_errors.begin(), frame_errors.end());
            if (worst_it == frame_errors.end() || *worst_it <= options.max_frame_error)
            {
                break;
            }

            const size_t worst_index = static_cast<size_t>(std::distance(frame_errors.begin(), worst_it));
            if (observations.size() - 1 < static_cast<size_t>(options.min_frames))
            {
                break;
            }

            std::cout << "dropping outlier " << observations[worst_index].image_path.filename().string()
                      << " with error " << *worst_it << " px\n";
            observations.erase(observations.begin() + static_cast<std::ptrdiff_t>(worst_index));
        }

        const fs::path output_path = fs::absolute(options.output);
        save_calibration_json(output_path, image_size, rms, camera_matrix, dist_coeffs);

        std::cout << "saved calibration to " << output_path << '\n';
        std::cout << "rms reprojection error: " << rms << '\n';
        std::cout << "camera matrix:\n" << camera_matrix << '\n';
        std::cout << "distortion coefficients:\n" << dist_coeffs.t() << '\n';
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
