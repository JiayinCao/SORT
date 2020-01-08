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

#include "direct.h"
#include "integratormethod.h"
#include "math/intersection.h"
#include "core/scene.h"
#include "light/light.h"
#include "core/memory.h"
#include "sampler/sampler.h"

SORT_STATS_DECLARE_COUNTER(sPrimaryRayCount)

SORT_STATS_COUNTER("Direct Illumination", "Primary Ray Count" , sPrimaryRayCount);
IMPLEMENT_RTTI( DirectLight );

Spectrum DirectLight::Li( const Ray& r , const PixelSample& ps , const Scene& scene ) const{
    SORT_STATS(++sPrimaryRayCount);

    if( r.m_Depth > max_recursive_depth )
        return 0.0f;

    // get the intersection between the ray and the scene
    Intersection ip;
    // evaluate light directly
    if( false == scene.GetIntersect( r , ip ) )
        return scene.Le( r );

    auto li = ip.Le( -r.m_Dir );

    // eavluate direct light
    auto light_num = scene.LightNum();
    for( auto i = 0u ; i < light_num ; ++i ){
        const auto light = scene.GetLight(i);
        li += EvaluateDirect( r , scene , light , ip , LightSample(true) , BsdfSample(true) , true );
    }

    return li;
}