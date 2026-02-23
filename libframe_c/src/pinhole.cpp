#include <pinhole.h>
#include <cmath>

PinholeCamera::PinholeCamera(const Eigen::Vector3d &eye,
                             const Eigen::Vector3d &lookat,
                             const Eigen::Vector3d &up,
                             double fov,
                             double aspect)
    : origin(eye)
{
    // viewport related calculations
    // this defines details about image plane.
    double theta = fov * M_PI / 180.0;
    double h = std::tan(theta / 2.0);
    double viewport_height = 2.0 * h;
    double viewport_width = aspect * viewport_height;

    // details the camera coordinate system
    Eigen::Vector3d w = (eye - lookat).normalized(); // camera direction
    Eigen::Vector3d u = up.cross(w).normalized();    // right
    Eigen::Vector3d v = w.cross(u);                  // up corrected

    // viewport size in 3D space
    horizontal = u * viewport_width;
    vertical = v * viewport_height;
    lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
}

Ray PinholeCamera::getRay(double u, double v) const
{
    // this is like reversing ray from center of camera to the specific pixel
    Eigen::Vector3d dir = lower_left_corner + u * horizontal + v * vertical - origin;
    return {origin, dir.normalized()};
}

void PinholeCamera::updateCamera(const Eigen::Vector3d &eye,
                                 const Eigen::Vector3d &lookat,
                                 const Eigen::Vector3d &up,
                                 double fov,
                                 double aspect)
{
    origin = eye;
    double theta = fov * M_PI / 180.0;
    double h = tan(theta / 2);
    double viewport_height = 2.0 * h;
    double viewport_width = aspect * viewport_height;

    Eigen::Vector3d w = (eye - lookat).normalized();
    Eigen::Vector3d u = up.cross(w).normalized();
    Eigen::Vector3d v = w.cross(u);

    horizontal = viewport_width * u;
    vertical = viewport_height * v;
    lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
}
