/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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
#include "math/utils.h"
#include "memory.h"
#include "core/rand.h"
#include "core/memory.h"
#include "core/scene.h"

float FresnelMoment1(const float eta) {
    float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta, eta5 = eta4 * eta;
    if (eta < 1)
        return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 + 2.49277f * eta4 - 0.68441f * eta5;
    return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 - 1.27198f * eta4 + 0.12746f * eta5;
}

SeparableBssrdf::SeparableBssrdf( const Intersection* intersection , const float ior_i , const float ior_e )
    : Bssrdf( ior_i , ior_e ) , intersection(intersection) {
    nn = Normalize(intersection->normal);
    btn = Normalize(Cross( nn , intersection->tangent ));
    tn = Normalize(Cross( btn , nn ));
}

Spectrum SeparableBssrdf::S( const Vector& wo , const Point& po , const Vector& wi , const Point& pi ) const{
    const auto F = DielectricFresnel(CosTheta(wo), ior_e, ior_i);
    return (1 - F) * Sr( Distance( po , pi ) ) * Sw(wi);
}

Spectrum SeparableBssrdf::Sw( const Vector& wi ) const{
    const auto F = DielectricFresnel(CosTheta(wi), ior_e, ior_i);
    const auto c = 1.0f - 2.0f * FresnelMoment1( ior_e / ior_i );
    return (1 - F) / (c * PI);
}

void SeparableBssrdf::Sample_S( const Scene& scene , const Vector& wo , const Point& po , BSSRDFIntersections& inter ) const {
    Vector vx , vy , vz;
    const auto r0 = sort_canonical();

    if( r0 <= 0.5f ){
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

    const auto ch = clamp( (int)(sort_canonical() * SPECTRUM_SAMPLE) , 0 , 2 );

    const auto tmp = sort_canonical();
    const auto r = Sample_Sr(ch, tmp);

    // This is essentialy rejection sampling on top of inverse importance sampling, since the rejection region is fairly small, 
    // there shouldn't be a performance problem.
    if( UNLIKELY( r < 0.0f ) )
        return;

    const auto rMax = fmax( 0.015f , Max_Sr(ch) );
    const auto l = 2.0f * sqrt( SQR(rMax) - SQR(r) );

    const auto phi = TWO_PI * sort_canonical();
    const auto source = po + r * ( vx * cos(phi) + vz * sin(phi) ) + l * vy * 0.5f;
    const auto target = source - l * vy;

    auto current = source;

    while( inter.cnt < TOTAL_INTERSECTION_CNT ){
        const auto t = Dot( current - target , vy );
        if( t <= 0 )
            break;

        // Instead of doing this, a better approach is to update the spatial data structure to support multi-intersection tests.
        // And don't look for intersections with primitives with other materials, a better approach may be to look for other primitives in the same sub-mesh.
        // However, since SORT supports more than triangle primitives, there is no mesh concecpt for other primitive types. In order to keep it a general
        // algorithm, it looks for primitives with same material.
        auto* pIntersection = SORT_MALLOC(BSSRDFIntersection)();
        Ray r( current , -vy , 0 , 0.01f , t );
        if( !scene.GetIntersect( r , &pIntersection->intersection , intersection->primitive->GetMaterial()->GetID() ) )
            break;
        
		const auto bssrdf = Sr( Distance( po , pIntersection->intersection.intersect ) );
        const auto pdf = Pdf_Sp( po , pIntersection->intersection.intersect , pIntersection->intersection.gnormal );
        if( pdf > 0.0f && !bssrdf.IsBlack() ){
            // update intersection
            pIntersection->weight = bssrdf / pdf;
            inter.intersections[inter.cnt++] = pIntersection;
        }

        // update original of the next ray
        current = pIntersection->intersection.intersect;
    }
}

float SeparableBssrdf::Pdf_Sp( const Point& po , const Point& pi , const Vector& n ) const {
    Vector d = po - pi;
    Vector dLocal( Dot( btn , d ) , Dot( nn , d ) , Dot( tn , d ) );
    Vector nLocal( Dot( btn , n ) , Dot( nn , n ) , Dot( tn , n ) );

    float rProj[3] = { sqrt( SQR( dLocal.y ) + SQR( dLocal.z ) ) ,
                       sqrt( SQR( dLocal.x ) + SQR( dLocal.z ) ) ,
                       sqrt( SQR( dLocal.x ) + SQR( dLocal.y ) ) };

    constexpr float axisProb[3] = { 0.25f , 0.5f , 0.25f };
    constexpr auto chProb = 1.0f / (float)SPECTRUM_SAMPLE;
    auto pdf = 0.0f ;
    for( auto axis = 0 ; axis < 3 ; ++axis ){
        for( auto ch = 0 ; ch < SPECTRUM_SAMPLE ; ++ch )
            pdf += Pdf_Sr( ch , rProj[axis] ) * std::abs( nLocal[axis] ) * axisProb[axis];
    }
    pdf *= chProb;
    return pdf;
}

SeparableBssrdfAdapter::SeparableBssrdfAdapter( const SeparableBssrdf* bssrdf )
:m_bssrdf(bssrdf),Bxdf( WHITE_SPECTRUM , BXDF_REFLECTION , DIR_UP , true ){
}

Spectrum SeparableBssrdfAdapter::f( const Vector& wo , const Vector& wi ) const{
    return m_bssrdf->Sw( wi ) * saturate( CosTheta( wi ) );
}
