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

#include "core/define.h"
#include "math/vector3.h"
#include "core/rand.h"
#include "math/matrix.h"
#include "math/utils.h"
#include "core/samplemethod.h"

//! @brief  Phase function interface.
/**
 * Phase function is the analog of BXDF in the context of volumetric rendering.
 * Unlike BXDF, integration of phase function along all directions needs to be exactly 1.
 */
class PhaseFunction{
public:
    //! @brief  Evaluation of the phase function.
    //!
    //! Just like BXDF definition, the directions passed in need to point from outside from
    //! the same point of interest.
    //!
    //! @param  wo      Out-going direction.
    //! @param  wi      Incoming direction.
    //! @return         Evaluation of phase function.
    virtual float P( const Vector& wo , const Vector& wi ) const = 0;

    //! @brief  Sample an incoming direction based on out-going direction.
    //!
    //! @param wo       Out-going direction.
    //! @param wi       Incoming direction.
    //! @param pdf      Pdf of sampling the incoming direction.
    virtual void    Sample( const Vector& wo , Vector& wi , float& pdf ) const = 0;
};

//! @brief  The Henyey-Greenstein phase function.
/**
 * This phase function is a one dimensional function that only considers the angle formed by 
 * the out-going and incoming directions. It exists in isotropic medium.
 * 
 * The Henyey-Greenstein phase function.
 * https://www.astro.umd.edu/~jph/HG_note.pdf
 */
class HenyeyGreenstein : public PhaseFunction{
public:
    //! @brief  Constructor.
    //!
    //! @param  g   Asymmetry parameter in Henyey-Greenstein phase function.
    HenyeyGreenstein( const float g ):g(g),sqrG(SQR(g)),twoG(2.0f*g){
    }

    //! @brief  Evaluation of the phase function.
    //!
    //! Just like BXDF definition, the directions passed in need to point from outside from
    //! the same point of interest.
    //!
    //! @param  wo      Out-going direction.
    //! @param  wi      Incoming direction.
    //! @return         Evaluation of phase function.
    float P( const Vector& wo , const Vector& wi ) const override{
        return evaluate( wo , wi );
    }

    //! @brief  Sample an incoming direction based on out-going direction.
    //!
    //! This is a perfect probability density function that is 
    //! @param wo       Out-going direction.
    //! @param wi       Incoming direction.
    //! @param pdf      Pdf of sampling the incoming direction.
    void    Sample( const Vector& wo , Vector& wi , float& pdf ) const override{
        if( fabs(g) > 0.03f ){
            // p(cos_theta) = ( 1 + g^2 - ( ( 1 - g^2 ) / ( 1 - g + 2 * g * t ) )^2 ) / ( 2 * g )
            const auto r = sort_canonical();
            const auto cos_theta = ( 1.0f + sqrG - SQR( ( 1.0f - sqrG ) / ( 1 - g + twoG * r ) ) ) / ( twoG );
            const auto sin_theta = ssqrt( 1.0f - SQR( cos_theta ) );

            const auto phi = TWO_PI * sort_canonical();
            const auto tmp = sphericalVec( sin_theta , cos_theta , phi );

            Vector t0, t1;
            coordinateSystem( wo, t0, t1 );
            Matrix m(t0.x, wo.x, t1.x, 0.0f,
                     t0.y, wo.y, t1.y, 0.0f,
                     t0.z, wo.z, t1.z, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f);
            wi = m.TransformVector(tmp);

            pdf = evaluate( wo , wi );
        }else{
            // there is no need to transform the incoming vector like the other branch since it is totally isotropic.
            const auto u = sort_canonical();
            const auto v = sort_canonical();
            wi = UniformSampleSphere(u, v);
            pdf = UniformSpherePdf();
        }
    }

private:
    const float g;
    const float sqrG;
    const float twoG;

    SORT_FORCEINLINE float evaluate( const Vector& wo , const Vector& wi ) const{
        const auto cos_theta = dot( wo , wi );
        const auto denom = 1.0f + sqrG - twoG * cos_theta;
        return INV_FOUR_PI * ( 1 - sqrG ) / ( denom * sqrt( denom ) );
    }
};