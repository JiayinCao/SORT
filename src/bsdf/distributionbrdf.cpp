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

#include "distributionbrdf.h"
#include "core/samplemethod.h"
#include "bsdf/fresnel.h"
#include "sampler/sample.h"

// The scaling factor used in the original paper.
constexpr static float A = 4.0f;

Spectrum DistributionBRDF::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto OoN = CosTheta( wo );
    const auto IoN = CosTheta( wi );
    const auto wh = Normalize( wo + wi );
    const auto IoH = Dot( wi , wh );

    // Crafting a Next-Gen Material Pipeline for The Order: 1886, Eq. 22
    const auto dterm = [&]( const Vector& wh ){
        return ( 1.0f + A * exp( -1.0f / ( TanTheta2(wh) * alphaSqr ) ) / Pow<4>( SinTheta(wh) ) ) / ( PI * ( 1.0f + A * alphaSqr ) );
    };

    const auto F = SchlickFresnel( specular , IoH );
    const auto SR = slerp( 1.0f , R , specularTint );
    return slerp( R * INV_PI , SR * dterm(wh) / ( 4.0f * ( OoN + IoN - OoN * IoN ) ) , F ) * IoN;
}

Spectrum DistributionBRDF::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const{
    //return Bxdf::sample_f( wo , wi , bs , pPdf );
    const auto v = sort_canonical() * ( 1.0f + A * alphaSqr );

    // The equation to importance sample the pdf is
    //  epsilon = ( sin(\theta)^2 + A * alpha^2 * exp( ( 1.0f - 1.0f / sin(\theta)^2 ) ) / alpha^2 ) ) / ( 1 + A * alpha^2 )
    // Unfortunately, there is no analytical solver. However, since the function is mono-increasing, a binary search can be used
    // as an alternative, not the optimal solution, but better than nothing.
    auto l = 0.0f , r = 1.0f;
    while( fabs( r - l ) > 0.001f ){
        const auto m = ( l + r ) * 0.5f;
        const auto t = m + A * alphaSqr * exp( ( 1 - 1.0f / m ) / alphaSqr );
        if( t < v )
            l = m;
        else
            r = m;
    }
    const auto sin_theta = sqrt( l );
    const auto cos_theta = sqrt( 1.0f - l );
    const auto phi = TWO_PI * sort_canonical();
    const auto wh = SphericalVec( sin_theta , cos_theta , phi );
    wi = reflect( wo , wh );

    if( pPdf )
        *pPdf = pdf( wo , wi );

    return f( wo , wi );
}

float DistributionBRDF::pdf(const Vector& wo, const Vector& wi) const{
    //return Bxdf::pdf( wo , wi );
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto wh = Normalize( wo + wi );
    const auto OoH = Dot( wo , wh );

    // Pdf of wh w.r.t solid angle
    //   Pdf(wh) = ( 1.0f + A * exp( -1.0f / ( alpha^2 * tan_theta(wh)^2 ) ) / sin_theta(wh)^4 ) * cos_theta( wh ) / ( ( 1.0f + A * alpha^2 ) * PI )
    const auto p = ( 1.0f + A * exp( -1.0f / ( TanTheta2( wh ) * alphaSqr ) / Pow<4>( SinTheta(wh) ) ) ) * CosTheta( wh ) / ( ( 1.0f + A * alphaSqr ) * PI );
    return p / ( 4.0f * OoH );
}