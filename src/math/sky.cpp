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

#include "sky.h"
#include "math/ray.h"
#include "core/samplemethod.h"
#include "core/memory.h"

// evaluate value from sky
Spectrum Sky::Evaluate( const Vector& wi ) const
{
    float theta = sphericalTheta( wi );
    float phi = sphericalPhi( wi );

    float v = theta * INV_PI;
    float u = phi * INV_TWOPI;

    return m_sky.GetColorFromUV( u , 1.0f - v );
}

// get the average radiance
Spectrum Sky::GetAverage() const
{
    return m_sky.GetAverage();
}

// generate 2d distribution
void Sky::_generateDistribution2D()
{
    auto nu = m_sky.GetWidth();
    auto nv = m_sky.GetHeight();
    sAssert( nu != 0 && nv != 0 , LIGHT );
    auto data = std::make_unique<float[]>(nu*nv);
    for( auto i = 0 ; i < nv ; i++ )
    {
        auto offset = i * nu;
        float sin_theta = sin( (float)i / (float)nv * PI );

        for( auto j = 0 ; j < nu ; j++ )
            data[offset+j] = std::max( 0.0f , m_sky.GetColor( (int)j , (int)i ).GetIntensity() * sin_theta );
    }

    distribution.reset();
    distribution = std::make_unique<Distribution2D>( data.get() , nu , nv );
}

// sample direction
Vector Sky::sample_v( float u , float v , float* pdf , float* area_pdf ) const
{
    sAssert( distribution != 0 , LIGHT );

    float uv[2] ;
    float apdf = 0.0f;
    distribution->SampleContinuous( u , v , uv , &apdf );
    if( area_pdf ) *area_pdf = apdf;
    if( apdf == 0.0f )
        return Vector();

    float theta = PI * ( 1.0f - uv[1] );
    float phi = TWO_PI * uv[0];

    Vector wi = sphericalVec( theta , phi );
    if( pdf )
    {
        *pdf = apdf;
        float sin_theta = sinTheta( wi );
        if( sin_theta != 0.0f )
            *pdf /= TWO_PI * PI * sinTheta( wi );
        else
            *pdf = 0.0f;
    }

    return wi;
}

// get the pdf
float Sky::Pdf( const Vector& lwi ) const
{
    float sin_theta = sinTheta(lwi);
    if( sin_theta == 0.0f ) return 0.0f;
    float u , v;
    float theta = sphericalTheta( lwi );
    float phi = sphericalPhi( lwi );
    v = 1.0f - theta * INV_PI;
    u = phi * INV_TWOPI;

    return distribution->Pdf( u , v ) / ( TWO_PI * PI * sin_theta );
}
