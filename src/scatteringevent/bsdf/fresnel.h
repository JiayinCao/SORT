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

#include "spectrum/spectrum.h"
#include "math/utils.h"

// Memo on Fresnel equations
// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/

//! @brief          Helper function to evaluate the part of Schlick's Fresnel approximation.
//!
//! @param   cos    Cosine of the angle between the incident direction and normal.
//! @return         Evaluated fresnel weight.
SORT_FORCEINLINE float SchlickWeight(float cos) {
    return Pow<5>(saturate(1.0f - cos));
}

//! @brief          Evaluate schlick's fresnel approximation.
//!
//! @param  F0      Expected fresnel at perpendicular angle.
//! @param  cos     Cosine of the angle between the incident direction and normal.
//! @return         Evaluated fresnel value.
SORT_FORCEINLINE Spectrum SchlickFresnel( const Spectrum& F0 , float cos ){
    return F0 + SchlickWeight(cos) * ( Spectrum( 1.0f ) - F0 );
}
SORT_FORCEINLINE float SchlickFresnel(const float F0, float cos) {
    return F0 + SchlickWeight(cos) * ( 1.0f - F0 );
}

//! @brief      Dielectric fresnel.
//!
//! @param  cosI    Cosine of the angle between the incident direction and normal.
//!                 It could be negative, meaning the ray is leaving the surface from inside.
//! @param  eta_i   Index of refraction above the surface.
//! @param  eta_t   Index of refraction below the surface.
//! @return         Evaluated fresnel.
SORT_FORCEINLINE float DielectricFresnel( float cosI , float eta_i , float eta_t ){
    const bool entering = cosI > 0.0f;
    const float _etaI = entering ? eta_i : eta_t;
    const float _etaT = entering ? eta_t : eta_i;

    const float sinI = sqrt(saturate(1.0f - SQR(cosI)));
    const float sinT = _etaI * sinI / _etaT;
    if( sinT >= 1.0f ) return 1.0f;
    if( !entering ) cosI = -cosI;

    const float cosT = sqrt( 1.0f - sinT * sinT );

    const float t0 = _etaT * cosI;
    const float t1 = _etaI * cosT;
    const float t2 = _etaI * cosI;
    const float t3 = _etaT * cosT;

    const float Rparl = ( t0 - t1 ) / ( t0 + t1 );
    const float Rparp = ( t2 - t3 ) / ( t2 + t3 );
    return ( Rparl * Rparl + Rparp * Rparp ) * 0.5f;
}

SORT_FORCEINLINE Spectrum ConductorFresnel( float cosI , const Spectrum& eta , const Spectrum& k ){
    float sq_cos = cosI * cosI;

    Spectrum t = 2 * eta * cosI;
    Spectrum tmp_f = eta*eta+k*k;
    Spectrum tmp = tmp_f * sq_cos;
    Spectrum Rparl2 = (tmp - t + 1 ) / ( tmp + t + 1 );
    Spectrum Rperp2 = (tmp_f - t + sq_cos)/(tmp_f + t + sq_cos );

    return (Rparl2+Rperp2)*0.5f;
}

//! @brief Interface for fresnel.
class   Fresnel{
public:
    //! @brief Evaluate the Fresnel term.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
    virtual Spectrum Evaluate( float cosI ) const = 0;
};

//! @brief A hack that presents no fresnel.
class   FresnelNo : public Fresnel{
public:
    //! @brief Evaluate the Fresnel term.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
    Spectrum Evaluate( float cosI ) const override{ return 1.0f; }
};

//! @brief Fresnel for conductors.
class   FresnelConductor : public Fresnel{
public:
    //! Constructor
    //! @param  e   Index of refraction of the material.
    //! @param  kk  Absorption coefficient.
    FresnelConductor( const Spectrum& e , const Spectrum& kk ): eta(e) , k(kk) {}

    //! @brief Evaluate the Fresnel term.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
    Spectrum Evaluate( float cosI ) const override{
        return ConductorFresnel( cosI , eta , k );
    }

private:
    Spectrum eta;   /**< Index of refraction of the material. */
    Spectrum k;     /**< Absorption coefficient. */
};

//! @brief Fresnel for dielectric.
class   FresnelDielectric : public Fresnel{
public:
    //! Constructor
    //! @param  ei      Index of refraction of the medium on the side normal points.
    //! @param  et      Index of refraction of the medium on the other side normal points.
    FresnelDielectric( float ei , float et ): eta_t(et),eta_i(ei) {}

    //! @brief Evaluate the Fresnel term.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal.
    //! @return         Evaluated fresnel value.
    Spectrum Evaluate( float cosI ) const override{
        return DielectricFresnel( cosI , eta_i , eta_t );
    }

private:
    float eta_t;    /**< Index of refraction of the medium on the side normal points. */
    float eta_i;    /**< Index of refraction of the medium on the other side normal points. */
};

//! @brief Schlick Fresnel Approximation
template<class T>
class   FresnelSchlick : public Fresnel{
public:
    //! Constructor
    //! @param  ei      Index of refraction of the medium on the side normal points.
    //! @param  et      Index of refraction of the medium on the other side normal points.
    FresnelSchlick( const T& F0 ) : F0(F0) {}

    //! @brief Evaluate the Fresnel term.
    //! @param cosI     Absolute cosine value of the angle between the incident ray and the normal. Caller of this function has to make sure cosI >= 0.0f.
    //! @return         Evaluated fresnel value.
    Spectrum Evaluate(float cosI) const override{
        return SchlickFresnel(F0, cosI);
    }

private:
    const T F0;
};