/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "math/point.h"
#include "math/ray.h"
#include "spectrum/spectrum.h"
#include "math/vector2.h"
#include "math/matrix.h"

class PixelSample;
class Visibility;
struct SurfaceInteraction;
struct RenderContext;

//! @brief Abstract camera
/**
 * This class serves as an abstract interface for different camera model.
 * There are several derived classes, such as perspective camera, orthogonal
 * camera and environment camera.
 */
class   Camera{
public:
    //! @brief Virtual destructor.
    virtual ~Camera() {}

    //! @brief Camera pre-proessing.
    //!
    //! Camera will do some pre-processing after camera intialization once all properties have been set.
    virtual void PreProcess() {}

    //! @brief Generating a primary ray.
    //! @param x    Coordinate along horizontal axis on the image sensor, it could be a float value.
    //! @param y    Coordinate along vertical axis on the image sensor, it could be a float value.
    //! @param ps   Pixel sample holding several useful random variables.
    //! @return     The generated ray based on the input.
    virtual Ray GenerateRay( float x, float y, const PixelSample& ps) const = 0;

    //! @brief      Get viewing direction.
    //!
    //! @return     Camera forward direction in world space.
    virtual Vector GetForward() const = 0;

    //! @brief Get camera coordinate according to a view direction in world space. It is used in light tracing or bi-directional path tracing algorithm.
    //! @param inter            The intersection to be considered when randomly sampling a point on the sensor.
    //! @param pdfw             PDF w.r.t the solid angle of choosing the direction.
    //! @param pdfa             PDF w.r.t the area of choosing the viewing point.
    //! @param cosAtCamera      The cosine factor of the angle between the viewing direction and forward direction.
    //! @param we               The importance function.
    //! @param eyeP             The selected random viewing point in world space.
    //! @param visibility       The structure holding visibility information.
    //! @return                 The coordinate on the image sensor. Its values range from 0 to width/height - 1.
    virtual Vector2i GetScreenCoord(const SurfaceInteraction& inter, float* pdfw, float* pdfa, float& cosAtCamera , Spectrum* we ,
                                    Point* eyeP , Visibility* visibility, RenderContext& rc) const = 0;

protected:
    Point           m_eye;                      /**< Viewing point of the camera. */
    float           m_sensorW = 0.0f;           /**< Image sensor width. */
    float           m_sensorH = 0.0f;           /**< Image sensor height. */
    float           m_aspectRatioW = 0.0f;      /**< Aspect ratio along x axis. */
    float           m_aspectRatioH = 0.0f;      /**< Aspect ratio along y axis. */
    int             m_aspectFit = 0;            /**< Aspect fit. It equals to 1 if it fits horizontally, otherwise it is 2. */
};
