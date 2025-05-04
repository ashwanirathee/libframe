#include <Eigen/Dense>
#include "ray.h"

bool hitSphere(const Eigen::Vector3d &center, double radius, const Ray &ray)
{
    Eigen::Vector3d oc = ray.origin - center;
    double a = ray.direction.dot(ray.direction);
    double b = 2.0 * oc.dot(ray.direction);
    double c = oc.dot(oc) - radius * radius;
    double discriminant = b * b - 4 * a * c;
    return (discriminant >= 0);
}

Eigen::Vector3d rayColor(const Ray &r)
{
    if (hitSphere(Eigen::Vector3d(0, 0, -1), 0.5, r))
        return Eigen::Vector3d(1, 0, 1); // Red sphere

    // Background gradient
    Eigen::Vector3d unit_direction = r.direction.normalized();
    double t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * Eigen::Vector3d(1.0, 1.0, 1.0) + t * Eigen::Vector3d(0.5, 0.7, 1.0);
}
