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

#include "pathtracing.h"
#include "math/intersection.h"
#include "bsdf/bsdf.h"
#include "bssrdf/bssrdf.h"
#include "core/scene.h"
#include "integratormethod.h"
#include "camera/camera.h"
#include "core/log.h"
#include "core/profile.h"
#include "bsdf/lambert.h"

SORT_STATS_DEFINE_COUNTER(sTotalPathLength)
SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Path Tracing", "Primary Ray Count" , sPrimaryRayCount);
SORT_STATS_AVG_COUNT("Path Tracing", "Average Length of Path", sTotalPathLength , sPrimaryRayCount);    // This also counts the case where ray hits sky

IMPLEMENT_RTTI( PathTracing );

Spectrum PathTracing::Li( const Ray& ray , const PixelSample& ps , const Scene& scene ) const{
    return li( ray , ps , scene );
}

Spectrum PathTracing::li( const Ray& ray , const PixelSample& ps , const Scene& scene , int bounces , bool indirectOnly , int bssrdfBounces , bool replaceSSS ) const{
    SORT_PROFILE("Path tracing");
    SORT_STATS(++sPrimaryRayCount);

    Spectrum    L = 0.0f;
    Spectrum    throughput = 1.0f;

    int local_bounce = 0;
    auto    r = ray;
    while(true){
        // This introduces bias in the algorithm. 'max_recursive_depth' could be set very large to reduce the side-effect.
        if( bounces >= max_recursive_depth )
            return L;

        SORT_STATS(++sTotalPathLength);

        // get the intersection between the ray and the scene if it's a light , accumulate the radiance and break
        Intersection inter;
        if( !scene.GetIntersect( r , &inter ) ){
            if( 0 == local_bounce )
                return !indirectOnly ? scene.Le( r ) : 0.0f;
            break;
        }

        if( local_bounce == 0 && !indirectOnly ) 
            L += inter.Le(-r.m_Dir);
        
        // make sure there is intersected primitive
        sAssert( nullptr != inter.primitive , INTEGRATOR );

        // the lack of multiple bounces between different BSSRDF surfaces does introduce a bias.
        replaceSSS |= ( bssrdfBounces > m_maxBouncesInBSSRDFPath - 1 );

        // evaluate the light
        Bsdf*   bsdf = nullptr;
        Bssrdf* bssrdf = nullptr;
        inter.primitive->GetMaterial()->UpdateScattering(inter, bsdf, bssrdf , replaceSSS );
        auto        light_pdf = 0.0f;
        const auto  light_sample = LightSample(true);
        const auto  bsdf_sample = BsdfSample(true);
        const auto  light = scene.SampleLight( light_sample.t , &light_pdf );
        if( light_pdf > 0.0f )
            L += throughput * EvaluateDirect( bsdf , r , scene , light, inter , light_sample , bsdf_sample , BXDF_TYPE(BXDF_ALL) ) / light_pdf;;
        
        // sample the next direction using bsdf
        float       path_pdf;
        Vector      wi;
        BXDF_TYPE   bxdf_type;
        Spectrum f;
        BsdfSample  _bsdf_sample = BsdfSample(true);
        f = bsdf->sample_f( -r.m_Dir , wi , _bsdf_sample , &path_pdf , BXDF_ALL , &bxdf_type );
        if( ( f.IsBlack() || path_pdf == 0.0f ) && !bsdf->SamplingSSS() )
            break;

        // update path weight
        if( !bsdf->SamplingSSS() )
            throughput *= f / path_pdf;

        if( 0.0f == throughput.GetIntensity() )
            break;

        // handle BSSRDF here
        
        if( bssrdf && bsdf->SamplingSSS() ){
            BSSRDFIntersections bssrdf_inter;
            bssrdf->Sample_S( scene, -r.m_Dir, inter.intersect, bssrdf_inter);

            const auto& intersection = inter;

            // Accumulate the contribution from direct illumination
            Spectrum total_bssrdf = 0.0f;
            if( bssrdf_inter.cnt > 0 ){
                Spectrum total_bssrdf;

                for( auto i = 0 ; i < bssrdf_inter.cnt ; ++i ){
                    const auto& pInter = bssrdf_inter.intersections[i];
                    const auto& intersection = pInter->intersection;

					// The base color is terribly wrong, it is clearly a bug, to be fixed later.
                    // Create a temporary lambert model to account the cos factor
                    // Fresnel is totally ignored here due to two reasons, the lack of visual differences and most importantly,
                    // there will be a discontinuity introduced when mean free path approaches zero.
                    bsdf = SORT_MALLOC(Bsdf)(&pInter->intersection);
                    bsdf->AddBxdf( SORT_MALLOC(Lambert)( WHITE_SPECTRUM , FULL_WEIGHT , DIR_UP ) );

                    // Accumulate the contribution from direct illumination
                    total_bssrdf += SampleOneLight( bsdf , r , intersection , scene ) * pInter->weight;

                    // Counts the light from indirect illumination recursively
                    float pdf = 0.0f;
                    BXDF_TYPE   dummy;
                    Spectrum f = bsdf->sample_f( -r.m_Dir, wi, BsdfSample(true), &pdf, BXDF_ALL, &dummy);
                    if( !f.IsBlack() && pdf > 0.0f && !pInter->weight.IsBlack() )
                        total_bssrdf += li( Ray( intersection.intersect , wi , 0 , 0.0001f ) , PixelSample() , scene , bounces + 1 , true , bssrdfBounces + 1 , true ) * f * pInter->weight / pdf;
                }
                
                L += total_bssrdf * throughput / path_pdf;
            }
            return L;
        }else{
            r.m_Ori = inter.intersect;
            r.m_Dir = wi;
            r.m_fMin = 0.0001f;
        }

        if( bounces > 3 && throughput.GetMaxComponent() < 0.1f ){
            auto continueProperbility = std::max( 0.05f , 1.0f - throughput.GetMaxComponent() );
            if( sort_canonical() < continueProperbility )
                break;
            throughput /= 1 - continueProperbility;
        }

        ++bounces;
        
        ++local_bounce;

        replaceSSS = false;
    }

    return L;
}
