#ifndef PINHOLE_HPP
#define PINHOLE_HPP

#include <Eigen/Dense>
#include "ray.h"

// eye and origin are essentially the same thing
// lookat is the point in space that the camera is looking at
// up is the up vector of the camera
// fov is the field of view in degrees
// aspect is the aspect ratio of the camera
class PinholeCamera
{
public:
    PinholeCamera(const Eigen::Vector3d &eye,
                  const Eigen::Vector3d &lookat,
                  const Eigen::Vector3d &up,
                  double fov,
                  double aspect);

    void updateCamera(const Eigen::Vector3d &eye,
                      const Eigen::Vector3d &lookat,
                      const Eigen::Vector3d &up,
                      double fov,
                      double aspect);
    Ray getRay(double u, double v) const;
    // Getters
    const Eigen::Vector3d &getOrigin() const { return origin; }
    const Eigen::Vector3d &getHorizontal() const { return horizontal; }
    const Eigen::Vector3d &getVertical() const { return vertical; }
    const Eigen::Vector3d &getLowerLeftCorner() const { return lower_left_corner; }

    // Setters
    void setOrigin(const Eigen::Vector3d &o) { origin = o; }
    void setHorizontal(const Eigen::Vector3d &h) { horizontal = h; }
    void setVertical(const Eigen::Vector3d &v) { vertical = v; }
    void setLowerLeftCorner(const Eigen::Vector3d &llc) { lower_left_corner = llc; }

private:
    Eigen::Vector3d origin;
    Eigen::Vector3d horizontal;
    Eigen::Vector3d vertical;
    Eigen::Vector3d lower_left_corner;
};

#endif