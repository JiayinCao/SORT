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

#include "bssrdf.h"
#include "math/intersection.h"
#include "bsdf/fresnel.h"
#include "bsdf/bsdf.h"
#include "core/rand.h"
#include "math/utils.h"
#include "core/scene.h"

float FresnelMoment1(const float eta) {
    float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta, eta5 = eta4 * eta;
    if (eta < 1)
        return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 + 2.49277f * eta4 - 0.68441f * eta5;
    return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 - 1.27198f * eta4 + 0.12746f * eta5;
}

SeperableBssrdf::SeperableBssrdf( const Intersection* intersection , const float ior_i , const float ior_e ): Bssrdf( ior_i , ior_e ) {
    nn = Normalize(intersection->normal);
	btn = Normalize(Cross( nn , intersection->tangent ));
	tn = Normalize(Cross( btn , nn ));
}

Spectrum SeperableBssrdf::S( const Vector& wo , const Point& po , const Vector& wi , const Point& pi ) const{
    const auto F = DielectricFresnel(CosTheta(wo), ior_e, ior_i);
    return (1 - F) * Sr( Distance( po , pi ) ) * Sw(wi);
}

Spectrum SeperableBssrdf::Sw( const Vector& wi ) const{
    const auto F = DielectricFresnel(CosTheta(wi), ior_e, ior_i);
    const auto c = FresnelMoment1( ior_e / ior_i );
    return (1 - F) / (c * PI);
}

Spectrum SeperableBssrdf::Sample_S( const Scene& scene , const Vector& wo , const Point& po , Vector& wi , Point& pi ) const {
    Vector vx , vy , vz;
    const auto r0 = sort_canonical();
    if( r0 < 0.5f ){
        vx = btn;
        vy = nn;
        vz = tn;
    }else if( r0 < 0.75f ){
        vx = tn;
        vy = btn;
        vz = nn;
    }else{
        vx = nn;
        vy = tn;
        vz = btn;
    }

    const auto ch = clamp( 0 , 2 , (int)(sort_canonical() * SPECTRUM_SAMPLE) );

    const auto r = Sample_Sr(ch, sort_canonical());
    if( r < 0.0f ) return 0.0f;
    const auto phi = TWO_PI * sort_canonical();

    const auto rMax = Sample_Sr(ch, 0.9999f);
    if( r >= rMax ) return 0.0f;
    const auto l = 2 * sqrt( SQR(rMax) - SQR(r) );

    const auto source = po + r * ( vx * cos(phi) + vz * sin(phi) ) - l * vy * 0.5f;
    const auto target = source + l * vy;

    auto current = source;

    while( true ){
        const auto t = Dot( current - target , vy );
        if( t <= 0 )
            break;

        Ray r( current , -vy , 0 , 0.001f , t );
        Intersection intersect;
        if( !scene.GetIntersect( r , &intersect ) )
            break;

        current = intersect.intersect;
    }

    return 1.0f;
}