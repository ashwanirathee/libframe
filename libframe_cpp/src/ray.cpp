#include <Eigen/Dense>
#include "ray.h"
#include <iostream>

bool hitSphere(const Eigen::Vector3d &center, double radius, const Ray &ray)
{
    Eigen::Vector3d oc = ray.origin - center;
    double a = ray.direction.dot(ray.direction);
    double b = 2.0 * oc.dot(ray.direction);
    double c = oc.dot(oc) - radius * radius;
    double discriminant = b * b - 4 * a * c;

    // this is simply puting the line equation in sphere equation to see there
    // are any feasible solutions
    // if there are any solutions, then the ray hits the sphere
    return (discriminant >= 0);
}

Eigen::Vector3d ray_color(const Ray &r, double v)
{
    // // assuming ray hits, then we send the color of the sphere
    if (hitSphere(Eigen::Vector3d(0, 0, -1), 0.1, r)){
        std::cout << "hit sphere" << std::endl;
        return Eigen::Vector3d(0, 0, 1); // blue sphere
    }
        
        
    return Eigen::Vector3d(1.0, 1.0, 1.0) * v; // white background
}