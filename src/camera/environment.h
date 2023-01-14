/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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

#include "camera.h"

//! @brief Environment camera.
/**
 * Environment camera capture everything around the viewing point.
 */
class   EnvironmentCamera : public Camera{
public:
    //! @brief Generating a primary ray.
    //! @param x    Coordinate along horizontal axis on the image sensor, it could be a float value.
    //! @param y    Coordinate along vertical axis on the image sensor, it could be a float value.
    //! @param ps   Pixel sample holding several useful random variables.
    //! @return     The generated ray based on the input.
    Ray GenerateRay( float x , float y , const PixelSample& ps ) const override;

    //! @brief Get camera coordinate according to a view direction in world space. It is used in light tracing or bi-directional path tracing algorithm.
    //! @param inter            The intersection to be considered when randomly sampling a point on the sensor.
    //! @param pdfw             PDF w.r.t the solid angle of choosing the direction.
    //! @param pdfa             PDF w.r.t the area of choosing the viewing point.
    //! @param cosAtCamera      The cosine factor of the angle between the viewing direction and forward direction.
    //! @param we               The importance function.
    //! @param eyeP             The selected random viewing point in world space.
    //! @param visibility       The structure holding visibility information.
    //! @return                 The coordinate on the image sensor. Its values range from 0 to width/height - 1.
    Vector2i GetScreenCoord(const SurfaceInteraction& inter, float* pdfw, float* pdfa, float& cosAtCamera , Spectrum* we ,
                                    Point* eyeP , Visibility* visibility, RenderContext& rc) const override {
        sAssert(false, GENERAL);
        return Vector2i();
    }

    //! @brief Get viewing direction.
    //! @return Camera forward direction.
    Vector GetForward() const override{
        sAssert(false, GENERAL);
        return Vector();
    }

private:
    /**< World transformation. */
    Matrix  m_transform;
};
