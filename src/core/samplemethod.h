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

#include <vector>
#include <algorithm>
#include "core/define.h"
#include "texture/texturebase.h"
#include "core/sassert.h"
#include "scatteringevent/bsdf/bxdf_utils.h"

/*
description :
    There are some basic sampling methods used for Monte Carlo ray tracing.
*/

// sampling a point on unit disk uniformly using Shirley's Mapping
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
// para 'x' : x position on the unit disk
// para 'y' : y position on the unit disk
SORT_FORCEINLINE void UniformSampleDisk( float u , float v , float& x , float& y ){
    float r , theta;
    float su = 2.0f * u - 1.0f;
    float sv = 2.0f * v - 1.0f;
    float asu = fabs( su );
    float asv = fabs( sv );

    if( asu < asv ){
        r = asv;
        float factor = ( sv > 0.0f ) ? -1.0f : 1.0f;
        theta = factor * su / r + 4.0f + 2.0f * factor ;
    }else if( asv < asu ){
        r = asu;
        float factor = ( su > 0.0f ) ? 1.0f : -1.0f;
        theta = factor * sv / r - 2.0f * factor + 2.0f  ;
        if( theta < 0.0f )
            theta += 8.0f;
    }else{
        x = 0.0f;
        y = 0.0f;
        return;
    }

    theta *= PI / 4.0f;

    x = cos( theta ) * r;
    y = sin( theta ) * r;
}

// sampling a vector in a hemisphere using cosine pdf
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
SORT_FORCEINLINE Vector CosSampleHemisphere( float u , float v ){
    float x , z ;
    UniformSampleDisk( u , v , x , z );
    float y = sqrt( std::max( 0.0f , 1.0f - x * x - z * z ) );
    return Vector( x , y , z );
}

// cosine hemisphere pdf
SORT_FORCEINLINE float CosHemispherePdf( const Vector& v ){
    return absCosTheta(v) * INV_PI;
}

// sampling a cone uniformly
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
SORT_FORCEINLINE Vector UniformSampleCone( float u , float v , float cos_max ){
    sAssert( cos_max <= 1.0f && cos_max >= -1.0f , SAMPLING );

    float cos_theta = ( 1.0f - u ) + u * cos_max;
    float sin_theta = sqrt( 1.0f - cos_theta * cos_theta );
    float phi = TWO_PI * v;

    return Vector( cosf(phi)*sin_theta , cos_theta , sinf(phi)*sin_theta );
}

// sampling a cone uniformly
SORT_FORCEINLINE float UniformConePdf( float cos_max ){
    return 1.0f / ( TWO_PI * ( 1.0f - cos_max ) );
}

// sampling a vector in a hemisphere uniformly
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
SORT_FORCEINLINE Vector UniformSampleHemisphere( float u , float v ){
    float theta = acos( u );
    float phi = TWO_PI * v;

    return sphericalVec( theta , phi );
}

// uniformly sample hemisphere pdf
SORT_FORCEINLINE float UniformHemispherePdf(){
    return INV_TWOPI;
}

// sampling a vector in sphere uniformly
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
SORT_FORCEINLINE Vector UniformSampleSphere( float u , float v ){
    float theta = acos( 1 - 2.0f * u );
    float phi = TWO_PI * v;
    return sphericalVec( theta , phi );
}

// pdf of uniformly sampling a vector on sphere
SORT_FORCEINLINE float UniformSpherePdf(){
    return INV_FOUR_PI;
}

// one dimensional distribution
class Distribution1D{
public:
    // constructor
    Distribution1D( const float* f , unsigned n ):
        count(n)
    {
        cdf = 0;
        sum = 0.0f;
        if( f == 0 || n == 0 )
            return;

        cdf = std::make_unique<float[]>(n + 1);
        cdf[0] = 0;
        for( unsigned i = 0 ; i < n ; i++ )
            cdf[i+1] = cdf[i] + f[i];
        sum = cdf[n];

        if( sum != 0.0f )
            for( unsigned i = 0 ; i < n+1 ; ++i )
                cdf[i] /= sum;
        else
            for( unsigned i = 0 ; i < n+1 ; ++i )
                cdf[i] = (float)i / (float)(n);
    }

    // get a discrete sample
    // para 'u' : a canonical random variable
    // para 'pdf' : probability density function value for the sample
    // result   : corresponding bucket straddle the u, -1 if there is no data in the distribution
    int SampleDiscrete( float u , float* pdf ) const{
        sAssert( count != 0 && cdf != 0 , SAMPLING );
        sAssert( u <= 1.0f && u >= 0.0f , SAMPLING );

        float* target = std::lower_bound( cdf.get() , cdf.get() + count + 1 , u );
        unsigned offset = (u<=0.0f)? 0:(int)(target-cdf.get()-1);
        // special care needs to be payed to situation when u == 0.0f
        if( offset == 0 ){
            while( offset < count && cdf[offset+1] == 0.0f )
                offset++;
        }
        if( offset == count ){
            if( pdf ) *pdf = 0.0f;
            return 0;
        }
        if( pdf )
            *pdf = cdf[offset+1] - cdf[offset];
        return offset;
    }

    // get a continuous sample
    // para 'u' : a canonical random variable
    // para 'pdf' : property density function value for the sample
    float SampleContinuous( float u , float* pdf ) const{
        sAssert( count != 0 && cdf != 0 , SAMPLING );
        sAssert( u <= 1.0f && u >= 0.0f , SAMPLING );

        float* target = std::lower_bound( cdf.get() , cdf.get()+count+1 , u );
        unsigned offset = (u<=0.0f)?0:(int)(target-cdf.get()-1);
        // special care needs to be payed to situation when u == 0.0f
        if( offset == 0 )
        {
            while( offset < count && cdf[offset+1] == 0.0f )
                offset++;
        }
        if( offset == count )
        {
            if( pdf ) *pdf = 0.0f;
            return 0.0f;
        }
        if( pdf )
            *pdf = (cdf[offset+1]-cdf[offset]) * count;
        float du = ( u - cdf[offset] ) / ( cdf[offset+1] - cdf[offset] );
        return ( du + (float)offset ) / (float)count;
    }

    // get the sum of the original data
    float GetSum() const{
        return sum;
    }

    // get the count
    unsigned GetCount() const{
        return count;
    }

    // get property of the unit
    float GetProperty( unsigned i ) const{
        sAssert( i < count , GENERAL );
        return cdf[i+1]-cdf[i];
    }

private:
    const unsigned              count;
    std::unique_ptr<float[]>    cdf;
    float                       sum;
};

// two dimensional distribution
class Distribution2D{
public:
    // default constructor
    Distribution2D( const float* data , unsigned nu , unsigned nv ){
        _init( data , nu , nv );
    }
    Distribution2D( const Texture2DBase* tex ){
        auto nu = tex->GetWidth();
        auto nv = tex->GetHeight();
        sAssert( nu != 0 && nv != 0 , GENERAL );
        std::unique_ptr<float[]> data = std::make_unique<float[]>(nu*nv);
        for( auto i = 0 ; i < nv ; i++ )
        {
            auto offset = i * nu;
            for( auto j = 0 ; j < nu ; j++ )
                data[offset+j] = tex->GetColor( j , i ).GetIntensity();
        }
        _init( data.get() , nu , nv );
    }

    // get a sample point
    void SampleContinuous( float u , float v , float uv[2] , float* pdf ){
        float pdf0 , pdf1;
        uv[1] = marginal->SampleContinuous( v , &pdf1 );
        int vi = (int)(uv[1] * m_nv);
        if( vi > (int)(m_nv - 1) )
            vi = (int)(m_nv - 1);
        uv[0] = pConditions[vi]->SampleContinuous( u , &pdf0 );

        if( pdf )
            *pdf = pdf0 * pdf1;
    }
    // get pdf
    float Pdf( float u , float v ) const{
        u = clamp( u , 0.0f , 1.0f );
        v = clamp( v , 0.0f , 1.0f );

        int iu = (int)( u * m_nu );
        int iv = (int)( v * m_nv );
        if( pConditions[iv]->GetSum() * marginal->GetSum() == 0.0f )
            return 0.0f;
        return pConditions[iv]->GetProperty( iu ) * m_nu * marginal->GetProperty( iv ) * m_nv ;// ( pConditions[iv]->GetSum() * marginal->GetSum() );
    }

private:
    // the distribution in each row
    std::vector<std::unique_ptr<Distribution1D>>    pConditions;
    // the marginal sampling distribution
    std::unique_ptr<Distribution1D> marginal;
    // the size for the two dimensions
    unsigned m_nu , m_nv;

    // initialize data
    void _init( const float* data , unsigned nu , unsigned nv ){
        for( unsigned i = 0 ; i < nv ; i++ )
            pConditions.push_back( std::make_unique<Distribution1D>( &data[i*nu] , nu ) );
        std::unique_ptr<float[]> m = std::make_unique<float[]>(nv);
        for( unsigned i = 0 ; i < nv ; i++ )
            m[i] = pConditions[i]->GetSum();
        marginal = std::make_unique<Distribution1D>( m.get() , nv );
        m_nu = nu;
        m_nv = nv;
    }
};
