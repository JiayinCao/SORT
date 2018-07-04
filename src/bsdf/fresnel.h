/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

#include "spectrum/spectrum.h"

//! @brief Interface for fresnel.
class	Fresnel
{
public:
	//! @brief Evaluate the Fresnel term.
    //! @param cosO     Absolute cosine value of the angle between the exit ray and the normal. Caller of this function has to make sure cosO >= 0.0f.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
	virtual Spectrum Evaluate( float cosO, float cosI ) const = 0;
};

//! @brief A hack that presents no fresnel.
class	FresnelNo : public Fresnel
{
public:
    //! @brief Evaluate the Fresnel term.
    //! @param cosO     Absolute cosine value of the angle between the exit ray and the normal. Caller of this function has to make sure cosO >= 0.0f.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
    Spectrum Evaluate( float cosO, float cosI ) const override{ return 1.0f; }
};

//! @brief Fresnel for conductors.
class	FresnelConductor : public Fresnel
{
public:
	//! Constructor
    //! @param  e   Index of refraction of the material.
    //! @param  kk  Absorption coefficient.
	FresnelConductor( const Spectrum& e , const Spectrum& kk ): eta(e) , k(kk) {}

    //! @brief Evaluate the Fresnel term.
    //! @param cosO     Absolute cosine value of the angle between the exit ray and the normal. Caller of this function has to make sure cosO >= 0.0f.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
	Spectrum Evaluate( float cosO, float cosI ) const override
	{
		float sq_cos = cosI * cosI;

		Spectrum t = 2 * eta * cosI;
		Spectrum tmp_f = eta*eta+k*k;
		Spectrum tmp = tmp_f * sq_cos;
		Spectrum Rparl2 = (tmp - t + 1 ) / ( tmp + t + 1 );
		Spectrum Rperp2 = (tmp_f - t + sq_cos)/(tmp_f + t + sq_cos );

		return (Rparl2+Rperp2)*0.5f;
	}

private:
    Spectrum eta;   /**< Index of refraction of the material. */
    Spectrum k;     /**< Absorption coefficient. */
};

//! @brief Fresnel for dielectric.
class	FresnelDielectric : public Fresnel
{
public:
    //! Constructor
    //! @param  ei      Index of refraction of the medium on the side normal points.
    //! @param  et      Index of refraction of the medium on the other side normal points.
	FresnelDielectric( float ei , float et ): eta_t(et),eta_i(ei) {}

    //! @brief Evaluate the Fresnel term.
    //! @param cosO     Absolute cosine value of the angle between the exit ray and the normal. Caller of this function has to make sure cosO >= 0.0f.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
	Spectrum Evaluate( float cosO, float cosI ) const override
	{
		const float t0 = eta_t * cosI;
		const float t1 = eta_i * cosO;
		const float t2 = eta_i * cosI;
		const float t3 = eta_t * cosO;

		float Rparl = ( t0 - t1 ) / ( t0 + t1 );
		float Rparp = ( t2 - t3 ) / ( t2 + t3 );

		return ( Rparl * Rparl + Rparp * Rparp ) * 0.5f;
	}

private:
    float eta_t;    /**< Index of refraction of the medium on the side normal points. */
    float eta_i;    /**< Index of refraction of the medium on the other side normal points. */
};
