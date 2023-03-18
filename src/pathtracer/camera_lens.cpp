#include "camera.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "CGL/misc.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;

namespace CGL {

using Collada::CameraInfo;

Ray Camera::generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta) const {

  // TODO Project 3-2: Part 4
  // compute position and direction of ray from the input sensor sample coordinate.
  // Note: use rndR and rndTheta to uniformly sample a unit disk.
  /*
    double u = (2. * x - 1.) * tan(0.5 * radians(hFov));
    double v = (2. * y - 1.) * tan(0.5 * radians(vFov));
    Vector3D pLens(lensRadius * sqrt(rndR) * cos(rndTheta), lensRadius * sqrt(rndR) * sin(rndTheta),0);
    // get pFocus
    Vector3D pFocus(u * focalDistance, v * focalDistance, -focalDistance);
    // calculate the direction
    Vector3D direction = (pFocus - pLens).unit();
    Ray r = Ray(c2w * pLens + pos, c2w * direction, fClip);
    r.min_t = nClip;
    return r;*/
    double w_half = tan(radians(hFov) * 0.5), h_half = tan(radians(vFov) * 0.5);
    Vector3D ideal_dir = Vector3D(
        (2 * x - 1) * w_half,
        (2 * y - 1) * h_half,
        -1);

    Vector3D p_lens = lensRadius * sqrt(rndR) * Vector3D(cos(rndTheta), sin(rndTheta), 0);
    Vector3D p_focus = ideal_dir / ideal_dir.z * (-focalDistance);
    Vector3D dir = p_focus - p_lens;
    dir.normalize();

    Ray result(c2w * p_lens + pos, c2w * dir);
    result.min_t = nClip;
    result.max_t = fClip;
    return result;
}


} // namespace CGL
