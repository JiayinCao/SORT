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

#include "ir.h"
#include "integratormethod.h"
#include "math/intersection.h"
#include "core/scene.h"
#include "light/light.h"
#include "scatteringevent/scatteringevent.h"

SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)
SORT_STATS_DEFINE_COUNTER(sVPLCount)

SORT_STATS_COUNTER("Instant Radiosity", "Primary Ray Count" , sPrimaryRayCount);
SORT_STATS_COUNTER("Instant Radiosity", "Virtual Point Lights Count" , sVPLCount);

IMPLEMENT_RTTI( InstantRadiosity );

// Preprocess
void InstantRadiosity::PreProcess( const Scene& scene )
{
    SORT_PROFILE("Instant Radiosity (LPV distribution stage)");

    m_pVirtualLightSources = std::make_unique<std::list<VirtualLightSource>[]>(m_nLightPathSet);

    for( int k = 0 ; k < m_nLightPathSet ; ++k ){
        for( int i = 0 ; i < m_nLightPaths ; ++i ){
            // pick a light first
            float light_pick_pdf;
            const Light* light = scene.SampleLight( sort_canonical() , &light_pick_pdf );

            // sample a ray from the light source
            float   light_emission_pdf = 0.0f;
            float   light_pdfa = 0.0f;
            Ray     ray;
            float   cosAtLight = 1.0f;
            Spectrum le = light->sample_l( LightSample(true) , ray , &light_emission_pdf , &light_pdfa , &cosAtLight );

            Spectrum throughput = le * cosAtLight / ( light_pick_pdf * light_emission_pdf );

            int current_depth = 0;
            Intersection intersect;
            while( true ){
                if (false == scene.GetIntersect(ray, &intersect))
                    break;

                VirtualLightSource ls;
                ls.power = throughput;
                ls.intersect = intersect;
                ls.wi = -ray.m_Dir;
                ls.depth = ++current_depth;
                m_pVirtualLightSources[k].push_back( ls );

                float bsdf_pdf;
                Vector wo;
                
                ScatteringEvent se( intersect , SE_EVALUATE_ALL_NO_SSS );
                intersect.primitive->GetMaterial()->UpdateScatteringEvent(se);
                Spectrum bsdf_value = se.Sample_BSDF( ls.wi , wo, BsdfSample(true) , bsdf_pdf );

                if( bsdf_pdf == 0.0f )
                    break;

                // apply russian roulette
                float continueProperbility = std::min( 1.0f , throughput.GetIntensity() );
                if( sort_canonical() > continueProperbility )
                    break;
                throughput /= continueProperbility;

                // update throughput
                throughput *= bsdf_value / bsdf_pdf;

                // update next ray
                ray = Ray(intersect.intersect, wo, 0, 0.001f);
            }
        }

        SORT_STATS(sVPLCount+=m_pVirtualLightSources[k].size());
    }
}

// radiance along a specific ray direction
Spectrum InstantRadiosity::Li( const Ray& r , const PixelSample& ps  , const Scene& scene ) const{
    SORT_STATS( ++sPrimaryRayCount );
    return _li( r , scene );
}

// private method of li
Spectrum InstantRadiosity::_li( const Ray& r , const Scene& scene , bool ignoreLe , float* first_intersect_dist ) const{
    // return if it is larger than the maximum depth
    if( r.m_Depth > max_recursive_depth )
        return 0.0f;

    // get intersection from camera ray
    Intersection ip;
    if( false == scene.GetIntersect( r , &ip ) )
        return ignoreLe?0.0f:scene.Le( r );

    // evaluate light path less than two vertices
    Spectrum radiance = ignoreLe?0.0f:ip.Le( -r.m_Dir );
    unsigned light_num = scene.LightNum();
    for( unsigned i = 0 ; i < light_num ; ++i ){
        const auto light = scene.GetLight(i);
        radiance += EvaluateDirect( r , scene , light , ip , LightSample(true) , BsdfSample(true) , true );
    }

    if( first_intersect_dist )
        *first_intersect_dist = ip.t;

    // pick a virtual light source randomly
    const unsigned lps_id = std::min( m_nLightPathSet - 1 , (int)(sort_canonical() * m_nLightPathSet) );
    std::list<VirtualLightSource> vps = m_pVirtualLightSources[lps_id];

    ScatteringEvent se( ip , SE_EVALUATE_ALL_NO_SSS );
    ip.primitive->GetMaterial()->UpdateScatteringEvent(se);

    // evaluate indirect illumination
    Spectrum indirectIllum;
    std::list<VirtualLightSource>::const_iterator it = vps.begin();
    while( it != vps.end() ){
        if( r.m_Depth + it->depth > max_recursive_depth ){
            ++it;
            continue;
        }

        const auto  delta = ip.intersect - it->intersect.intersect;
        const auto  sqrLen = delta.SquaredLength();
        const auto  len = sqrt( sqrLen );
        const auto  n_delta = delta / len;

        ScatteringEvent se1( it->intersect , SE_EVALUATE_ALL_NO_SSS );
        it->intersect.primitive->GetMaterial()->UpdateScatteringEvent(se1);

        const auto    gterm = 1.0f / std::max( m_fMinSqrDist , sqrLen );
        const auto    f0 = se.Evaluate_BSDF( -r.m_Dir , -n_delta );
        const auto    f1 = se1.Evaluate_BSDF( n_delta , it->wi );

        Spectrum    contr = gterm * f0 * f1 * it->power;
        if( !contr.IsBlack() ){
            Visibility vis(scene);
            vis.ray = Ray( it->intersect.intersect , n_delta , 0 , 0.001f , len - 0.001f );

            if( vis.IsVisible() )
                indirectIllum += contr;
        }

        ++it;
    }
    radiance += indirectIllum / (float)m_nLightPaths;

    if( m_fMinDist > 0.0f ){
        Vector  wi;
        float   bsdf_pdf;
        const auto f = se.Sample_BSDF( -r.m_Dir , wi , BsdfSample( true ) , bsdf_pdf );

        if( !f.IsBlack() && bsdf_pdf != 0.0f ){
            PixelSample ps;
            float gather_dist;
            Ray gather_ray( ip.intersect , wi , r.m_Depth + 1 , 0.001f , m_fMinDist - 0.001f );
            Spectrum li = _li( gather_ray , scene , true , &gather_dist );

            if( !li.IsBlack() ){
                float dgterm = std::max( 0.0f , 1.0f - gather_dist * gather_dist / m_fMinSqrDist );
                radiance += f * li * dgterm / bsdf_pdf;
            }
        }
    }

    return radiance;
}