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

#include "skylight.h"
#include "core/rtti.h"
#include "math/sky.h"
#include "core/log.h"
#include "sampler/sample.h"

//! @brief  Definition of a captured sky light
class   HdrSkyLight : public SkyLight{
public:
    //! @brief  Given a ray, sample the light source if there is any intersection between the ray and the light source.
    //!
    //! It simply returns false for delta function, meaning there is no way to pick a ray hitting the delta light source.
    //! Integrator has to explicitly sample delta light sources instead of using this function and hoping to get radiances
    //! even if with a ray that accidentally hits the delta light sources.
    //!
    //! @param  ray             The ray to be evaluated.
    //! @param  intersect       The intersection between the ray and the light source.
    //! @param  radiance        The radiance goes from the light source to the ray origin.
    //! @return                 Whether there is an intersection between the ray and the light source.
    bool Le( const Ray& ray , SurfaceInteraction* intersect , Spectrum& radiance ) const override;

    //! @brief  The pdf w.r.t solid angle if the ray starting from 'p', tracing through 'wi' hits the light source.
    //!
    //! @param  p       The point in world space to be shaded.
    //! @param  wi      The direction pointing from the point.
    //! @return         The pdf w.r.t solid angle if the ray starting from 'p', tracing through 'wi' hits the light source.
    float Pdf( const Point& p , const Vector& wi ) const override;

    //! @brief  Get average sky color
    //!
    //! @return     Get The average sky color
    Spectrum GetAverage() const override{
        return sky.GetAverage() * intensity;
    }

    //! @brief  Load hdr image
    //!
    //! @param filepath  File Path of the Hdr image.
    bool LoadHdrImage(const std::string& filepath);

private:
    /**< Sky information. */
    Sky sky;

protected:
    //! @brief  Evalaute the radiance along a specific direction.
    //!
    //! @param dir   Direction pointing from the shading point to the sky
    //! @return      Radiance value of the sky along the input direction.
    Spectrum RadianceFromDirection( const Vector& dir ) const override{
        return sky.Evaluate( dir ) * intensity;
    }

    //! @brief  Sample a local direction in the light's local space
    //!
    //! @param ls           LightSample used to take the random sample
    //! @param pdf_w        Pdf w.r.t solid angle
    //! @param local_dir    Direction sampled in light's local space, pointing from the shaded point to the sky
    //! @param global_dir   Direction sampled in global sapce, pointing from the shaded point to the sky
    void   SampleLocalDirection(const LightSample& ls, float& pdf_w, Vector& local_dir, Vector& global_dir) const override{
        local_dir = sky.sample_v( ls.u , ls.v , &pdf_w , 0 );
        global_dir = m_light2world.TransformVector(local_dir);
    }

    //! @brief  Convert the direction from world space to local space
    //!
    //! WARNING!!!
    //! This function takes advantage of all its usage because the return value is merely used to retrieve the ambient light radiance or pdf.
    //! Since the radiance and pdf are always constant across all directions. There is no need to transform the direction for nothing.
    //! Global direciton is fine. However, if any code tries to treat the local directional as the real local direction, it will
    //! be fairly disappointed.
    //! 
    //! @param world_dir    Direction in world space.
    //! @return             Direction in local space.
    Vector LocalDirFromWorldDir(const Vector& world_dir) const override {
        return m_light2world.GetInversed().TransformVector(world_dir);
    }

    friend class SkyLightEntity;
};
