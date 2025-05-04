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

bool hitCircleWithUV(const Eigen::Vector3d &center,
    const Eigen::Vector3d &normal,
    double radius,
    const Ray &ray,
    double &u_angle)
{
Eigen::Vector3d N = normal.normalized();
double denom = ray.direction.dot(N);

if (std::abs(denom) < 1e-6) return false;

double t = (center - ray.origin).dot(N) / denom;
if (t < 0) return false;

Eigen::Vector3d P = ray.origin + t * ray.direction;
Eigen::Vector3d vec = P - center;

if (vec.squaredNorm() > radius * radius) return false;

// Get angle in the circle's local 2D space
Eigen::Vector3d x_axis = N.unitOrthogonal().normalized();     // local x
Eigen::Vector3d y_axis = N.cross(x_axis).normalized();        // local y

double x = vec.dot(x_axis);
double y = vec.dot(y_axis);
u_angle = std::atan2(y, x); // [-π, π]
return true;
}

Eigen::Vector3d ray_color(const Ray &r, double v)
{
    // // assuming ray hits, then we send the color of the sphere
    if (hitSphere(Eigen::Vector3d(0, 0, -1), 0.1, r))
        return Eigen::Vector3d(0, 0, 1); // blue sphere

    // Circle (Ashoka Chakra)
    double t = 0; // We'll compute t here as well
    double angle = 0;
    Eigen::Vector3d chakra_center(0.0, 0.0, -1.0);
    double chakra_radius = 0.30;

    Eigen::Vector3d N(0, 0, 1);
    double denom = r.direction.dot(N);
    if (std::abs(denom) >= 1e-6) {
        t = (chakra_center - r.origin).dot(N) / denom;
        if (t >= 0) {
            Eigen::Vector3d P = r.origin + t * r.direction;
            Eigen::Vector3d vec = P - chakra_center;
            double dist2 = vec.squaredNorm();

            if (dist2 <= chakra_radius * chakra_radius) {
                // Calculate angle
                Eigen::Vector3d x_axis = N.unitOrthogonal().normalized(); // local x
                Eigen::Vector3d y_axis = N.cross(x_axis).normalized();    // local y
                double x = vec.dot(x_axis);
                double y = vec.dot(y_axis);
                angle = fmod(std::atan2(y, x) + 2 * M_PI, 2 * M_PI); // Normalize angle to [0, 2π]

                // Check for spoke hit
                int spoke_count = 24;
                double spoke_width = 0.05;
                bool is_spoke = (fmod(angle, 2 * M_PI / spoke_count) < spoke_width);

                // Check for boundary ring
                double ring_thickness = 0.01;
                double radius2 = chakra_radius * chakra_radius;
                bool is_ring = (std::abs(dist2 - radius2) < 2 * chakra_radius * ring_thickness);

                if (is_spoke || is_ring)
                    return Eigen::Vector3d(0, 0, 1); // Blue
            }
        }
    }

    if (v > 0.68) {
        return Eigen::Vector3d(1.0, 0.5, 0.0); // Top - Orange
    } else if (v > 0.30) {
        return Eigen::Vector3d(1.0, 1.0, 1.0); // Middle - White
    } else {
        return Eigen::Vector3d(0.0, 0.5, 0.0); // Bottom - Green
    }
}