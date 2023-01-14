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
#include "medium/phasefunction.h"

SORT_STATS_DEFINE_COUNTER(sTotalPathLength)
SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Path Tracing", "Primary Ray Count" , sPrimaryRayCount);
SORT_STATS_AVG_COUNT("Path Tracing", "Average Length of Path", sTotalPathLength , sPrimaryRayCount);    // This also counts the case where ray hits sky

Spectrum PathTracing::Li( const Ray& ray , const PixelSample& ps , const Scene& scene, RenderContext& rc) const{
    MediumStack ms;
    scene.RestoreMediumStack(ray.m_Ori, rc, ms);

    return li( ray , ps , scene , 0 , false , 0 , false , ms , rc);
}

Spectrum PathTracing::li( const Ray& ray , const PixelSample& ps , const Scene& scene , int bounces , bool indirectOnly , int bssrdfBounces , bool replaceSSS , MediumStack& ms, RenderContext& rc ) const{
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
        if( !scene.GetIntersect( rc, r , inter ) ){
            if( 0 == local_bounce )
                return !indirectOnly ? scene.Le( r ) : 0.0f;
            break;
        }

        Spectrum emission;
        MediumInteraction* pMi = nullptr;
        const auto medium_attenuation = ms.Sample(r, inter.t, pMi, emission, rc);

        L += emission * throughput;

        // update the through put based on the medium attenuation due to particle scattering and absorption.
        throughput *= medium_attenuation;

        if (pMi && pMi->phaseFunction) {
            Vector wi;
            float pdf = 0.0f;
            const auto pf = pMi->phaseFunction->Sample(rc, -r.m_Dir, wi, pdf);

            if ( UNLIKELY(pdf == 0.0f) )
                break;

            // evaluate direct light illumination
            float light_pdf = 0.0f;
            const auto  light = scene.SampleLight(sort_rand<float>(rc), &light_pdf);
            L += throughput * EvaluateDirect(pMi->intersect, pMi->phaseFunction, -r.m_Dir, scene, light, ms, rc) / light_pdf;

            // update path weight
            throughput *= pf / pdf;

            if (0.0f == throughput.GetIntensity())
                break;

            r.m_Ori = pMi->intersect;
            r.m_Dir = wi;
            r.m_fMin = 0.0f;    // no need for bias anymore since there is no geometry

            // apply Prussian Roulette in volume scattering too
            if (bounces > 3 && throughput.GetMaxComponent() < 0.1f) {
                auto continueProperbility = std::max(0.05f, 1.0f - throughput.GetMaxComponent());
                if (sort_rand<float>(rc) < continueProperbility)
                    break;
                throughput /= 1 - continueProperbility;
            }

            ++bounces;
            ++local_bounce;

            continue;
        }

        if( local_bounce == 0 && !indirectOnly ) 
            L += inter.Le(-r.m_Dir);
        
        // make sure there is intersected primitive
        sAssert(IS_PTR_VALID(inter.primitive), INTEGRATOR );

        // the lack of multiple bounces between different BSSRDF surfaces does introduce a bias.
        replaceSSS |= ( bssrdfBounces > m_maxBouncesInBSSRDFPath - 1 );

        const MaterialBase* material = inter.primitive->GetMaterial();
        sAssert(IS_PTR_VALID(material), INTEGRATOR);

        // Parse the material and populate the results into a scatteringEvent.
        SE_Flag seFlag = replaceSSS ? SE_Flag( SE_EVALUATE_ALL | SE_REPLACE_BSSRDF ) : SE_EVALUATE_ALL;
        ScatteringEvent se(inter, seFlag);
        material->UpdateScatteringEvent(se, rc);

        SE_Flag scattering_type_flag;
        auto pdf_scattering_type = se.SampleScatteringType(rc, scattering_type_flag);

        if( scattering_type_flag & SE_EVALUATE_BXDF ){
            // evaluate the light
            auto        light_pdf = 0.0f;
            const auto  light_sample = LightSample(rc);
            const auto  bsdf_sample = BsdfSample(rc);
            const auto  light = scene.SampleLight( light_sample.t , &light_pdf );
            if( light_pdf > 0.0f )
                L += throughput * EvaluateDirect( se , r , scene, light , light_sample , bsdf_sample , material , ms , rc) / light_pdf / pdf_scattering_type;
        }else if(scattering_type_flag & SE_EVALUATE_BSSRDF) {
            BSSRDFIntersections bssrdf_inter;
            float               bssrdf_pdf = 0.0f;
            se.Sample_BSSRDF( scene, -r.m_Dir, se.GetInteraction().intersect, bssrdf_inter , bssrdf_pdf, rc);

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
                    se.AddBxdf( SORT_MALLOC(rc.m_memory_arena, Lambert)(rc, WHITE_SPECTRUM , FULL_WEIGHT , DIR_UP ) );

                    // Accumulate the contribution from direct illumination
                    total_bssrdf += SampleOneLight( se , r , intersection , scene , material , ms , rc ) * pInter->weight;
                }
                
                L += total_bssrdf * throughput / pdf_scattering_type / bssrdf_pdf;
            }
        }

        // pick another time for the next path
        pdf_scattering_type = se.SampleScatteringType(rc, scattering_type_flag);

        if( pdf_scattering_type == 0.0f )
            break;

        throughput /= pdf_scattering_type;
        if( scattering_type_flag & SE_EVALUATE_BXDF ){
            // sample the next direction using bsdf
            float       path_pdf;
            Vector      wi;
            Spectrum f;
            BsdfSample  _bsdf_sample = BsdfSample(rc);
            f = se.Sample_BSDF( -r.m_Dir , wi , _bsdf_sample , path_pdf, rc);
            if( ( f.IsBlack() || path_pdf == 0.0f ) )
                break;

            // as long as the ray is passing through the surface, it is necessary to update the medium stack.
            const auto interaction_flag = update_interaction_flag(dot(wi,inter.gnormal), dot(-r.m_Dir,inter.gnormal));
            if (SE_Interaction::SE_REFLECTION != interaction_flag) {
                MediumInteraction mi;
                mi.intersect = inter.intersect;
                mi.mesh = inter.primitive->GetMesh();
                material->UpdateMediumStack(mi, interaction_flag, ms, rc);
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
            se.Sample_BSSRDF( scene, -r.m_Dir, se.GetInteraction().intersect, bssrdf_inter , bssrdf_pdf, rc);

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
                    se.AddBxdf( SORT_MALLOC(rc.m_memory_arena, Lambert)(rc, WHITE_SPECTRUM , FULL_WEIGHT , DIR_UP ) );

                    // Counts the light from indirect illumination recursively
                    float pdf = 0.0f;
                    Vector wi;
                    Spectrum f = se.Sample_BSDF( -r.m_Dir, wi, BsdfSample(rc), pdf, rc);
                    if (!f.IsBlack() && pdf > 0.0f && !pInter->weight.IsBlack()) {
                        MediumStack ms_copy = ms;
                        total_bssrdf += li(Ray(intersection.intersect, wi, 0, 0.0001f), PixelSample(), scene, bounces + 1, true, bssrdfBounces + 1, true, ms_copy, rc) * f * pInter->weight / pdf;
                    }
                }
                
                L += total_bssrdf * throughput / bssrdf_pdf;
            }
            return L;
        }

        if( bounces > 3 && throughput.GetMaxComponent() < 0.1f ){
            auto continueProperbility = std::max( 0.05f , 1.0f - throughput.GetMaxComponent() );
            if( sort_rand<float>(rc) < continueProperbility )
                break;
            throughput /= 1 - continueProperbility;
        }

        ++bounces;
        ++local_bounce;

        replaceSSS = false;
    }

    return L;
}
