#ifndef RAY_H
#define RAY_H

#include <Eigen/Dense>

struct Ray
{
    Eigen::Vector3d origin;
    Eigen::Vector3d direction;
};

bool hitSphere(const Eigen::Vector3d &center, double radius, const Ray &ray);
Eigen::Vector3d ray_color(const Ray &r, double v);

#endif