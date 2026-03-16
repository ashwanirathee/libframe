#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
struct MappedBuffer
{
    void *start = MAP_FAILED;
    size_t length = 0;
};

struct CameraNode
{
    std::string device_path;
    std::string card;
    std::string bus_info;
    std::vector<__u32> formats;
};

int xioctl(int fd, unsigned long request, void *arg)
{
    while (true)
    {
        int rc = ioctl(fd, request, arg);
        if (rc != -1 || errno != EINTR)
        {
            return rc;
        }
    }
}

std::string fourcc_to_string(__u32 format)
{
    char s[5];
    s[0] = static_cast<char>(format & 0xFF);
    s[1] = static_cast<char>((format >> 8) & 0xFF);
    s[2] = static_cast<char>((format >> 16) & 0xFF);
    s[3] = static_cast<char>((format >> 24) & 0xFF);
    s[4] = '\0';
    return std::string(s);
}

bool supports_format(const CameraNode &node, __u32 format)
{
    return std::find(node.formats.begin(), node.formats.end(), format) != node.formats.end();
}

int node_score(const CameraNode &node)
{
    if (supports_format(node, V4L2_PIX_FMT_MJPEG))
    {
        return 2;
    }
    if (supports_format(node, V4L2_PIX_FMT_YUYV))
    {
        return 1;
    }
    return 0;
}

std::vector<__u32> query_formats(int fd)
{
    std::vector<__u32> formats;
    v4l2_fmtdesc desc {};
    desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (xioctl(fd, VIDIOC_ENUM_FMT, &desc) == 0)
    {
        formats.push_back(desc.pixelformat);
        ++desc.index;
    }

    return formats;
}

bool try_probe_node(const std::string &device_path, CameraNode &out_node)
{
    int fd = open(device_path.c_str(), O_RDWR);
    if (fd < 0)
    {
        return false;
    }

    v4l2_capability caps {};
    if (xioctl(fd, VIDIOC_QUERYCAP, &caps) < 0)
    {
        close(fd);
        return false;
    }

    if ((caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 ||
        (caps.capabilities & V4L2_CAP_STREAMING) == 0)
    {
        close(fd);
        return false;
    }

    std::vector<__u32> formats = query_formats(fd);
    close(fd);

    if (formats.empty())
    {
        return false;
    }

    out_node.device_path = device_path;
    out_node.card = reinterpret_cast<const char *>(caps.card);
    out_node.bus_info = reinterpret_cast<const char *>(caps.bus_info);
    out_node.formats = std::move(formats);
    return true;
}

std::vector<CameraNode> discover_camera_nodes()
{
    std::vector<CameraNode> nodes;

    for (const auto &entry : std::filesystem::directory_iterator("/dev"))
    {
        if (!entry.is_character_file())
        {
            continue;
        }

        const std::string name = entry.path().filename().string();
        if (name.rfind("video", 0) != 0)
        {
            continue;
        }

        CameraNode node;
        if (try_probe_node(entry.path().string(), node))
        {
            nodes.push_back(std::move(node));
        }
    }

    std::sort(nodes.begin(), nodes.end(), [](const CameraNode &a, const CameraNode &b) {
        return a.device_path < b.device_path;
    });

    return nodes;
}

std::vector<CameraNode> select_one_node_per_camera(const std::vector<CameraNode> &nodes)
{
    std::map<std::string, CameraNode> selected;

    for (const auto &node : nodes)
    {
        auto it = selected.find(node.bus_info);
        if (it == selected.end())
        {
            selected.emplace(node.bus_info, node);
            continue;
        }

        if (node_score(node) > node_score(it->second) ||
            (node_score(node) == node_score(it->second) && node.device_path < it->second.device_path))
        {
            it->second = node;
        }
    }

    std::vector<CameraNode> deduped;
    for (const auto &pair : selected)
    {
        deduped.push_back(pair.second);
    }

    std::sort(deduped.begin(), deduped.end(), [](const CameraNode &a, const CameraNode &b) {
        return a.bus_info < b.bus_info;
    });

    return deduped;
}

std::string describe_formats(const CameraNode &node)
{
    std::ostringstream oss;
    for (size_t i = 0; i < node.formats.size(); ++i)
    {
        if (i != 0)
        {
            oss << ",";
        }
        oss << fourcc_to_string(node.formats[i]);
    }
    return oss.str();
}

__u32 choose_pixel_format(const CameraNode &node)
{
    if (supports_format(node, V4L2_PIX_FMT_MJPEG))
    {
        return V4L2_PIX_FMT_MJPEG;
    }
    if (supports_format(node, V4L2_PIX_FMT_YUYV))
    {
        return V4L2_PIX_FMT_YUYV;
    }
    return node.formats.front();
}

std::string output_extension(__u32 pixel_format)
{
    if (pixel_format == V4L2_PIX_FMT_MJPEG)
    {
        return ".jpg";
    }
    if (pixel_format == V4L2_PIX_FMT_YUYV)
    {
        return ".yuyv";
    }
    return ".bin";
}

std::string sanitize_name(std::string value)
{
    for (char &ch : value)
    {
        if (!std::isalnum(static_cast<unsigned char>(ch)))
        {
            ch = '_';
        }
    }
    return value;
}

std::filesystem::path save_frame(const CameraNode &node,
                                 __u32 pixel_format,
                                 const void *data,
                                 size_t bytes_used)
{
    const std::filesystem::path path =
        std::filesystem::current_path() /
        (sanitize_name(node.bus_info) + output_extension(pixel_format));

    std::ofstream output(path, std::ios::binary);
    if (!output)
    {
        throw std::runtime_error("failed to open output file " + path.string());
    }

    output.write(static_cast<const char *>(data), static_cast<std::streamsize>(bytes_used));
    if (!output)
    {
        throw std::runtime_error("failed to write frame to " + path.string());
    }

    return path;
}

void capture_one_frame(const CameraNode &node, uint32_t width, uint32_t height)
{
    int fd = open(node.device_path.c_str(), O_RDWR);
    if (fd < 0)
    {
        throw std::runtime_error("open failed for " + node.device_path + ": " + strerror(errno));
    }

    v4l2_format format {};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = choose_pixel_format(node);
    format.fmt.pix.field = V4L2_FIELD_ANY;

    if (xioctl(fd, VIDIOC_S_FMT, &format) < 0)
    {
        close(fd);
        throw std::runtime_error("VIDIOC_S_FMT failed for " + node.device_path + ": " + strerror(errno));
    }

    v4l2_requestbuffers request {};
    request.count = 1;
    request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_REQBUFS, &request) < 0 || request.count < 1)
    {
        close(fd);
        throw std::runtime_error("VIDIOC_REQBUFS failed for " + node.device_path + ": " + strerror(errno));
    }

    v4l2_buffer buffer {};
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = 0;

    if (xioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0)
    {
        close(fd);
        throw std::runtime_error("VIDIOC_QUERYBUF failed for " + node.device_path + ": " + strerror(errno));
    }

    MappedBuffer mapped;
    mapped.length = buffer.length;
    mapped.start = mmap(nullptr, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);
    if (mapped.start == MAP_FAILED)
    {
        close(fd);
        throw std::runtime_error("mmap failed for " + node.device_path + ": " + strerror(errno));
    }

    if (xioctl(fd, VIDIOC_QBUF, &buffer) < 0)
    {
        munmap(mapped.start, mapped.length);
        close(fd);
        throw std::runtime_error("VIDIOC_QBUF failed for " + node.device_path + ": " + strerror(errno));
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0)
    {
        munmap(mapped.start, mapped.length);
        close(fd);
        throw std::runtime_error("VIDIOC_STREAMON failed for " + node.device_path + ": " + strerror(errno));
    }

    if (xioctl(fd, VIDIOC_DQBUF, &buffer) < 0)
    {
        xioctl(fd, VIDIOC_STREAMOFF, &type);
        munmap(mapped.start, mapped.length);
        close(fd);
        throw std::runtime_error("VIDIOC_DQBUF failed for " + node.device_path + ": " + strerror(errno));
    }

    const std::filesystem::path saved_path =
        save_frame(node, format.fmt.pix.pixelformat, mapped.start, buffer.bytesused);

    std::cout << node.device_path
              << " card=" << node.card
              << " bus=" << node.bus_info
              << " " << format.fmt.pix.width << "x" << format.fmt.pix.height
              << " format=" << fourcc_to_string(format.fmt.pix.pixelformat)
              << " bytes=" << buffer.bytesused
              << " saved=" << saved_path.string() << '\n';

    xioctl(fd, VIDIOC_STREAMOFF, &type);
    munmap(mapped.start, mapped.length);
    close(fd);
}
} // namespace

int main(int argc, char **argv)
{
    uint32_t width = argc > 3 ? static_cast<uint32_t>(std::strtoul(argv[3], nullptr, 10)) : 640;
    uint32_t height = argc > 4 ? static_cast<uint32_t>(std::strtoul(argv[4], nullptr, 10)) : 480;

    try
    {
        if (argc > 1 && std::string(argv[1]) == "--list")
        {
            const auto nodes = discover_camera_nodes();
            const auto cameras = select_one_node_per_camera(nodes);

            for (const auto &camera : cameras)
            {
                std::cout << camera.device_path
                          << " card=" << camera.card
                          << " bus=" << camera.bus_info
                          << " formats=" << describe_formats(camera) << '\n';
            }

            return 0;
        }

        std::vector<CameraNode> cameras;
        if (argc > 2)
        {
            CameraNode cam0;
            CameraNode cam1;
            if (!try_probe_node(argv[1], cam0))
            {
                throw std::runtime_error("could not probe " + std::string(argv[1]));
            }
            if (!try_probe_node(argv[2], cam1))
            {
                throw std::runtime_error("could not probe " + std::string(argv[2]));
            }
            cameras.push_back(std::move(cam0));
            cameras.push_back(std::move(cam1));
        }
        else
        {
            cameras = select_one_node_per_camera(discover_camera_nodes());
            if (cameras.size() < 2)
            {
                throw std::runtime_error("found fewer than two capture-capable cameras under /dev/video*");
            }
        }

        capture_one_frame(cameras[0], width, height);
        capture_one_frame(cameras[1], width, height);
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
