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

#include "pathtracing.h"
#include "math/interaction.h"
#include "scatteringevent/bssrdf/bssrdf.h"
#include "core/scene.h"
#include "integratormethod.h"
#include "camera/camera.h"
#include "core/log.h"
#include "core/profile.h"
#include "scatteringevent/bsdf/lambert.h"
#include "scatteringevent/scatteringevent.h"
#include "medium/medium.h"

SORT_STATS_DEFINE_COUNTER(sTotalPathLength)
SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Path Tracing", "Primary Ray Count" , sPrimaryRayCount);
SORT_STATS_AVG_COUNT("Path Tracing", "Average Length of Path", sTotalPathLength , sPrimaryRayCount);    // This also counts the case where ray hits sky

IMPLEMENT_RTTI( PathTracing );

Spectrum PathTracing::Li( const Ray& ray , const PixelSample& ps , const Scene& scene, MediumStack& ms) const{
    return li( ray , ps , scene , 0 , false , 0 , false , ms );
}

Spectrum PathTracing::li( const Ray& ray , const PixelSample& ps , const Scene& scene , int bounces , bool indirectOnly , int bssrdfBounces , bool replaceSSS , MediumStack& ms ) const{
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
        SurfaceInteraction inter;
        if( !scene.GetIntersect( r , inter ) ){
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

        // Parse the material and populate the results into a scatteringEvent.
        SE_Flag seFlag = replaceSSS ? SE_Flag( SE_EVALUATE_ALL | SE_REPLACE_BSSRDF ) : SE_EVALUATE_ALL;
        ScatteringEvent se(inter, seFlag);
        inter.primitive->GetMaterial()->UpdateScatteringEvent(se);

        SE_Flag scattering_type_flag;
        auto pdf_scattering_type = se.SampleScatteringType(scattering_type_flag);

        if( scattering_type_flag & SE_EVALUATE_BXDF ){
            // evaluate the light
            auto        light_pdf = 0.0f;
            const auto  light_sample = LightSample(true);
            const auto  bsdf_sample = BsdfSample(true);
            const auto  light = scene.SampleLight( light_sample.t , &light_pdf );
            if( light_pdf > 0.0f )
                L += throughput * EvaluateDirect( se , r , scene, light , light_sample , bsdf_sample ) / light_pdf / pdf_scattering_type;
        }else{
            BSSRDFIntersections bssrdf_inter;
            float               bssrdf_pdf = 0.0f;
            se.Sample_BSSRDF( scene, -r.m_Dir, se.GetInteraction().intersect, bssrdf_inter , bssrdf_pdf);

            // Accumulate the contribution from direct illumination
            if( bssrdf_inter.cnt > 0 ){
                Spectrum total_bssrdf;

                for( auto i = 0u ; i < bssrdf_inter.cnt ; ++i ){
                    const auto& pInter = bssrdf_inter.intersections[i];
                    const auto& intersection = pInter->intersection;

                    // Create a temporary lambert model to account the cos factor
                    // Fresnel is totally ignored here due to two reasons, the lack of visual differences and most importantly,
                    // there will be a discontinuity introduced when mean free path approaches zero.
                    ScatteringEvent se(pInter->intersection);
                    se.AddBxdf( SORT_MALLOC(Lambert)( WHITE_SPECTRUM , FULL_WEIGHT , DIR_UP ) );

                    // Accumulate the contribution from direct illumination
                    total_bssrdf += SampleOneLight( se , r , intersection , scene ) * pInter->weight;
                }
                
                L += total_bssrdf * throughput / pdf_scattering_type / bssrdf_pdf;
            }
        }

        // pick another time for the next path
        pdf_scattering_type = se.SampleScatteringType(scattering_type_flag);

        if( pdf_scattering_type == 0.0f )
            break;

        throughput /= pdf_scattering_type;
        if( scattering_type_flag & SE_EVALUATE_BXDF ){
            // sample the next direction using bsdf
            float       path_pdf;
            Vector      wi;
            Spectrum f;
            BsdfSample  _bsdf_sample = BsdfSample(true);
            SE_Interaction interaction_flag;
            f = se.Sample_BSDF( -r.m_Dir , wi , _bsdf_sample , path_pdf , &interaction_flag);
            if( ( f.IsBlack() || path_pdf == 0.0f ) )
                break;

            if (SE_Interaction::SE_ENTERING == interaction_flag) {
                // adding the medium attached to the material to the medium stack

                // to be done
            } else if (SE_Interaction::SE_LEAVING == interaction_flag) {
                // removing the medium attached to the material to the medium stack

                // to be done
            }

            // update path weight
            throughput *= f / path_pdf;

            if( 0.0f == throughput.GetIntensity() )
                break;
            
            r.m_Ori = inter.intersect;
            r.m_Dir = wi;
            r.m_fMin = 0.0001f;
        }else{
            // Strictly speaking, it should consider the possibility of crossing a volume when exit from the other point of the SSS object.
            // This is not handled properly in SORT because it is considered ill-defined scene in this case.
            // In a nutshell, content creator should avoid putting SSS object across volumes.
            // It is totally possible to reconstruct the volume stack after exiting from the SSS surface, which will most likely incur more costs.

            BSSRDFIntersections bssrdf_inter;
            float               bssrdf_pdf = 0.0f;
            se.Sample_BSSRDF( scene, -r.m_Dir, se.GetInteraction().intersect, bssrdf_inter , bssrdf_pdf);

            // Accumulate the contribution from direct illumination
            if( bssrdf_inter.cnt > 0 ){
                Spectrum total_bssrdf;

                for( auto i = 0u ; i < bssrdf_inter.cnt ; ++i ){
                    const auto& pInter = bssrdf_inter.intersections[i];
                    const auto& intersection = pInter->intersection;

                    // Create a temporary lambert model to account the cos factor
                    // Fresnel is totally ignored here due to two reasons
                    //  - the lack of visual differences 
                    //  - more importantly, there will be a discontinuity introduced when mean free path approaches zero.
                    ScatteringEvent se(pInter->intersection, SE_Flag( SE_EVALUATE_ALL | SE_REPLACE_BSSRDF ));
                    se.AddBxdf( SORT_MALLOC(Lambert)( WHITE_SPECTRUM , FULL_WEIGHT , DIR_UP ) );

                    // Counts the light from indirect illumination recursively
                    float pdf = 0.0f;
                    Vector wi;
                    Spectrum f = se.Sample_BSDF( -r.m_Dir, wi, BsdfSample(true), pdf);
                    if (!f.IsBlack() && pdf > 0.0f && !pInter->weight.IsBlack()) {
                        MediumStack ms_copy = ms;
                        total_bssrdf += li(Ray(intersection.intersect, wi, 0, 0.0001f), PixelSample(), scene, bounces + 1, true, bssrdfBounces + 1, true, ms_copy) * f * pInter->weight / pdf;
                    }
                }
                
                L += total_bssrdf * throughput / bssrdf_pdf;
            }
            return L;
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
