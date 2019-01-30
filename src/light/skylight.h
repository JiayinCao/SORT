/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include "core/rtti.h"
#include "math/sky.h"
#include "core/log.h"

//! @brief	Definition of sky light.
class	SkyLight : public Light{
public:
	//! @brief	Sample a direction given the intersection.
	//!
	//! Given an intersection, do importance sampling to pick a direction from intersection to light source. 
	//! For some light sources, light point light, spot light and distant light, it is trival. However, it
	//! needs some decent algorithm to make it efficient for some other light sources like area light.
	//!
	//! @param	intersect		The information of intersection.
	//! @param	ls				The light sample information.
	//! @param	dirToLight		The resulting direction goes from the intersection to light source.
	//! @param	distance		The distance from the intersected point to the sampled point, which is the intersection 
	//!							between the out-going direction and the light source.
	//! @param	pdfw			The resulting pdf w.r.t solid angle to pick such a direction.
	//! @param	emissionPdf		The pdf w.r.t solid angle if such a direction and position ( which is the intersection 
	//!							between the resulting direction to the light source ) is picked by the light source.
	//! @param	cosAtLight		The cos of the angle between the light out-going direction, the opposite of 'dirToLight'.
	//! @param	visibility		The visibility data structured filled by the light source.
	//! @return					The radiance goes from the light source to the intersected point.
	Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const;

	//! @brief		Sample a point and light out-going direction.
	//!
	//! The difference of this version the the above one is there is no intersection data given.
	//!
	//! @param	ls				The light sample.
	//! @param	r				The resulting sampled ray.
	//! @param	pdfA			The pdf w.r.t area of picking such a light out-going ray. It is simply one for delta light.
	//! @param	cosAtLight		The cos of the angle between the light out-going direction, the opposite of 'dirToLight'.
	//! @return					The radiance goes from the light source to the intersected point.
	Spectrum sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const;

	//! @brief	Get the radiance light starting from the light source and ending at the intersection point.
	//!
	//! It simply returns zero for delta function, meaning there is no way to pick a ray hitting the delta light source.
	//! Integrator has to explicitly sample delta light sources instead of using this function and hoping to get radiances
	//! even if with a ray that accidentally hits the delta light sources.
	//!
	//! @param	intersect		The intersection information.
	//! @param	wo				The direction goes from the intersection to the light source.
	//! @param	directPdfA		The pdf w.r.t area to pick the point, intersection between the direction and the light source.
	//! @param	emissionPdf		The pdf w.r.t solid angle to pick to sample such a position and direction goes to the intersection.
	//! @return					The radiance goes from the light source to the intersection, black if there is no intersection.
	Spectrum Le( const Intersection& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const;

	//! @brief	Given a ray, sample the light source if there is any intersection between the ray and the light source.
	//!
	//! It simply returns false for delta function, meaning there is no way to pick a ray hitting the delta light source.
	//! Integrator has to explicitly sample delta light sources instead of using this function and hoping to get radiances
	//! even if with a ray that accidentally hits the delta light sources.
	//!
	//! @param	ray				The ray to be evaluated.
	//! @param	intersect		The intersection between the ray and the light source.
	//! @param	radiance		The radiance goes from the light source to the ray origin.
	//! @return					Whether there is an intersection between the ray and the light source.
	bool Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const;

	//! @brief	Whether the light is an infinite light source.
	//!
	//! @return	 	Whether the light is an infinite light.
	bool IsInfinite() const {
		return true;
	}

	//! @brief	Approximation of total power of the light.
	//!
	//! The reason it is just an approximation is because there are certain kinds of light
	//! whose power is hard to evaluate, like distant light, sky light. Since this value is
	//! only used to pick a light for importance sampling, it is fine to be biased.
	//!
	//! @return		Approximation of the light power.
	Spectrum Power() const;

	//! @brief	Whether the light is a delta light source.
	//!
	//! @return	 	Whether the light is a delta light.
	bool IsDelta() const {
		return false;
	}

	//! @brief	The pdf w.r.t solid angle if the ray starting from 'p', tracing through 'wi' hits the light source.
	//!
	//! @param	p		The point in world space to be shaded.
	//! @param	wi		The direction pointing from the point.
	//! @return			The pdf w.r.t solid angle if the ray starting from 'p', tracing through 'wi' hits the light source.
	float Pdf( const Point& p , const Vector& wi ) const;

private:
	/**< Sky information. */
	Sky	sky;

    friend class SkyLightEntity;
};
