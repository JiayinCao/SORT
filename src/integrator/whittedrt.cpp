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

#include "whittedrt.h"
#include "integratormethod.h"
#include "math/interaction.h"
#include "core/scene.h"
#include "light/light.h"
#include "core/log.h"
#include "scatteringevent/scatteringevent.h"

SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Whitted Ray Tracing", "Primary Ray Count" , sPrimaryRayCount);

Spectrum WhittedRT::Li( const Ray& r , const PixelSample& ps , const Scene& scene, RenderContext& rc) const{
    SORT_STATS(++sPrimaryRayCount);

    if( r.m_Depth > max_recursive_depth )
        return 0.0f;

    // get the intersection between the ray and the scene
    SurfaceInteraction ip;
    if( false == scene.GetIntersect( rc, r , ip ) )
        return scene.Le(r);

    Spectrum t;

    // no support for SSS in this integrator.
    ScatteringEvent se( ip , SE_EVALUATE_ALL_NO_SSS );
    ip.primitive->GetMaterial()->UpdateScatteringEvent(se, rc);

    // lights
    Visibility visibility(scene);
    auto lights = scene.GetLights();
    auto it = lights.begin();
    while( it != lights.end() ){
        // only delta light is evaluated
        if( (*it)->IsDelta() ){
            Vector  lightDir;
            float   pdf;
            Spectrum ld = (*it)->sample_l( ip.intersect , &ps.light_sample[0] , lightDir , 0 , &pdf , 0 , 0 , visibility );
            if( ld.IsBlack() ){
                it++;
                continue;
            }
            //Spectrum f = bsdf->f( -r.m_Dir , lightDir );
            Spectrum f = se.Evaluate_BSDF( -r.m_Dir , lightDir );
            if( f.IsBlack() ){
                it++;
                continue;
            }
#ifndef ENABLE_TRANSPARENT_SHADOW
            const auto visible = visibility.IsVisible();
            if( visible )
                t += (ld * f / pdf);
#else
            const auto attenuation = visibility.GetAttenuation(rc);
            if( !attenuation.IsBlack() )
                t += (ld * f * attenuation / pdf );
#endif
        }
        it++;
    }

    return t;
}
