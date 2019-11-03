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
#include "scatteringevent/bsdf/bsdf.h"
#include "math/utils.h"
#include "memory.h"
#include "core/rand.h"
#include "core/memory.h"
#include "core/scene.h"

#define WITH_MULTI_INTERSECTION_SUPPORT

SeparableBssrdf::SeparableBssrdf( const Spectrum& R , const Intersection* intersection , const Spectrum& ew , const float sw )
    : Bssrdf( ew , sw ) , R(R) , intersection(intersection) , channels(0) {
    nn = Normalize(intersection->normal);
    btn = Normalize(Cross( nn , intersection->tangent ));
    tn = Normalize(Cross( btn , nn ));
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

    const auto ch = Sample_Ch();
    const auto tmp = sort_canonical();
    const auto r = Sample_Sr(ch, tmp);

    // This is essentially rejection sampling on top of inverse importance sampling, since the rejection region is fairly small, 
    // there shouldn't be a performance problem.
    if( UNLIKELY( r < 0.0f ) )
        return;

    const auto rMax = fmax( 0.0015f , Max_Sr(ch) );
    const auto l = 2.0f * sqrt( SQR(rMax) - SQR(r) );

    const auto phi = TWO_PI * sort_canonical();
    const auto source = po + r * ( vx * cos(phi) + vz * sin(phi) ) + l * vy * 0.5f;
    
#ifdef WITH_MULTI_INTERSECTION_SUPPORT
    const Ray ray( source , -vy , 0 , 0.0001f , l );
    scene.GetIntersect( ray , inter , intersection->primitive->GetMaterial()->GetID() );

    for( auto i = 0u ; i < inter.cnt ; ++i ){
        sAssert( inter.intersections[i] != nullptr , MATERIAL );

        auto pIntersection = inter.intersections[i];
        const auto bssrdf = Sr( Distance( po , pIntersection->intersection.intersect ) );
        const auto pdf = Pdf_Sp( po , pIntersection->intersection.intersect , pIntersection->intersection.gnormal );
        if( pdf > 0.0f && !bssrdf.IsBlack() )
            pIntersection->weight = bssrdf / pdf * GetEvalWeight();
    }
#else
    // This code will be removed once I have confirmed the above interface works well, it exists now purely for verification purpose.
    auto current = source;
    
    while( inter.cnt < TOTAL_SSS_INTERSECTION_CNT ){
        const auto target = source - l * vy;
        const auto t = Dot( current - target , vy );
        if( t <= 0 )
            break;

        // Instead of doing this, a better approach is to update the spatial data structure to support multi-intersection tests.
        // And don't look for intersections with primitives with other materials, a better approach may be to look for other primitives in the same sub-mesh.
        // However, since SORT supports more than triangle primitives, there is no mesh concept for other primitive types. In order to keep it a general
        // algorithm, it looks for primitives with same material.
        auto* pIntersection = SORT_MALLOC(BSSRDFIntersection)();
        Ray r( current , -vy , 0 , 0.001f , t );
        if( !scene.GetIntersect( r , &pIntersection->intersection , intersection->primitive->GetMaterial()->GetID() ) )
            break;
        
		const auto bssrdf = Sr( Distance( po , pIntersection->intersection.intersect ) );
        const auto pdf = Pdf_Sp( po , pIntersection->intersection.intersect , pIntersection->intersection.gnormal );
        if( pdf > 0.0f && !bssrdf.IsBlack() ){
            // update intersection
            pIntersection->weight = bssrdf / pdf * GetEvalWeight();
            inter.intersections[inter.cnt++] = pIntersection;
        }

        // update original of the next ray
        current = pIntersection->intersection.intersect;
    }
#endif
}

float SeparableBssrdf::Pdf_Sp( const Point& po , const Point& pi , const Vector& n ) const {
    Vector d = po - pi;
    Vector dLocal( Dot( btn , d ) , Dot( nn , d ) , Dot( tn , d ) );
    Vector nLocal( Dot( btn , n ) , Dot( nn , n ) , Dot( tn , n ) );

    float rProj[3] = { sqrt( SQR( dLocal.y ) + SQR( dLocal.z ) ) ,
                       sqrt( SQR( dLocal.x ) + SQR( dLocal.z ) ) ,
                       sqrt( SQR( dLocal.x ) + SQR( dLocal.y ) ) };

    constexpr float axisProb[3] = { 0.25f , 0.5f , 0.25f };
    auto pdf = 0.0f ;
    for( auto axis = 0 ; axis < 3 ; ++axis ){
        for( auto ch = 0 ; ch < SPECTRUM_SAMPLE ; ++ch ){
            #ifdef SSS_REPLACE_WITH_LAMBERT
            if( R[ch] == 0.0f )
                continue;
            #endif
            pdf += Pdf_Sr( ch , rProj[axis] ) * std::abs( nLocal[axis] ) * axisProb[axis];
        }
    }
    pdf /= channels;
    return pdf;
}
