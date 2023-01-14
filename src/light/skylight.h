/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#include "light.h"
#include "core/log.h"

//! @brief  Definition of sky light.
/**
 * SkyLight is an abstruction of different kinds of sky lights.
 */
class   SkyLight : public Light{
public:
    //! @brief  Sample a direction given the intersection.
    //!
    //! Given an intersection, do importance sampling to pick a direction from intersection to light source.
    //! For some light sources, light point light, spot light and distant light, it is trival. However, it
    //! needs some decent algorithm to make it efficient for some other light sources like area light.
    //!
    //! @param  ip              The point where we are interested in shading at.
    //! @param  ls              The light sample information.
    //! @param  dirToLight      The resulting direction goes from the intersection to light source.
    //! @param  distance        The distance from the intersected point to the sampled point, which is the intersection
    //!                         between the out-going direction and the light source.
    //! @param  pdfw            The resulting pdf w.r.t solid angle to pick such a direction.
    //! @param  emissionPdf     The pdf w.r.t solid angle if such a direction and position ( which is the intersection
    //!                         between the resulting direction to the light source ) is picked by the light source.
    //! @param  cosAtLight      The cos of the angle between the light out-going direction, the opposite of 'dirToLight'.
    //! @param  visibility      The visibility data structured filled by the light source.
    //! @return                 The radiance goes from the light source to the intersected point.
    Spectrum sample_l(const Point& ip, const LightSample* ls, Vector& dirToLight, float* distance, float* pdfw, float* emissionPdf, float* cosAtLight, Visibility& visibility) const override;

    //! @brief      Sample a point and light out-going direction.
    //!
    //! The difference of this version the the above one is there is no intersection data given.
    //!
    //! @param  ls              The light sample.
    //! @param  r               The resulting sampled ray.
    //! @param  pdfA            The pdf w.r.t area of picking such a light out-going ray. It is simply one for delta light.
    //! @param  cosAtLight      The cos of the angle between the light out-going direction, the opposite of 'dirToLight'.
    //! @return                 The radiance goes from the light source to the intersected point.
    Spectrum sample_l( RenderContext& rc, const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const override;

    //! @brief  Get the radiance light starting from the light source and ending at the intersection point.
    //!
    //! It simply returns zero for delta function, meaning there is no way to pick a ray hitting the delta light source.
    //! Integrator has to explicitly sample delta light sources instead of using this function and hoping to get radiances
    //! even if with a ray that accidentally hits the delta light sources.
    //!
    //! @param  intersect       The intersection information.
    //! @param  wo              The direction goes from the light source to the intersecion, NOT the ray points to the light source!
    //! @param  directPdfA      The pdf w.r.t area to pick the point, intersection between the direction and the light source.
    //! @param  emissionPdf     The pdf w.r.t solid angle to pick to sample such a position and direction goes to the intersection.
    //! @return                 The radiance goes from the light source to the intersection, black if there is no intersection.
    Spectrum Le(const SurfaceInteraction& intersect, const Vector& wo, float* directPdfA, float* emissionPdf) const override;

    //! @brief  Whether the light is an infinite light source.
    //!
    //! @return     Whether the light is an infinite light.
    bool IsInfinite() const override{
        return true;
    }

    //! @brief  Approximation of total power of the light.
    //!
    //! The reason it is just an approximation is because there are certain kinds of light
    //! whose power is hard to evaluate, like distant light, sky light. Since this value is
    //! only used to pick a light for importance sampling, it is fine to be biased.
    //!
    //! @return     Approximation of the light power.
    Spectrum Power() const override{
        sAssert(IS_PTR_VALID(m_scene), LIGHT );
        const BBox box = m_scene->GetBBox();
        const float radius = (box.m_Max - box.m_Min).Length() * 0.5f;

        return radius * radius * PI * GetAverage();
    }

    //! @brief  Whether the light is a delta light source.
    //!
    //! @return     Whether the light is a delta light.
    bool IsDelta() const  override{
        return false;
    }

    //! @brief  Get the average color of the sky
    //!
    //! @return     Average color of the sky
    virtual Spectrum GetAverage() const = 0;

    friend class SkyLightEntity;

protected:
    //! @brief  Evalaute the radiance along a specific direction.
    //!
    //! @param dir   Direction pointing from the shading point to the sky
    //! @return      Radiance value of the sky along the input direction.
    virtual Spectrum RadianceFromDirection( const Vector& dir ) const = 0;

    //! @brief  Sample a local direction in the light's local space
    //!
    //! @param ls           LightSample used to take the random sample
    //! @param pdf_w        Pdf w.r.t solid angle
    //! @param local_dir    Direction sampled in light's local space, pointing from the shaded point to the sky
    //! @param global_dir   Direction sampled in global sapce, pointing from the shaded point to the sky
    virtual void   SampleLocalDirection(const LightSample& ls, float& pdf_w, Vector& local_dir, Vector& global_dir) const = 0;

    //! @brief  Convert the direction from world space to local space
    //!
    //! @param world_dir    Direction in world space.
    //! @return             Direction in local space.
    virtual Vector LocalDirFromWorldDir(const Vector& world_dir) const = 0;
};
