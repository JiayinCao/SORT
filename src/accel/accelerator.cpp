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

#include "accelerator.h"
#include "core/primitive.h"

SORT_STATS_DEFINE_COUNTER(sRayCount)
SORT_STATS_DEFINE_COUNTER(sShadowRayCount)
SORT_STATS_DEFINE_COUNTER(sIntersectionTest)

void Accelerator::computeBBox(){
    // reset bounding box
    m_bbox.InvalidBBox();

    // update bounding box again
    for( auto& primitive : *m_primitives )
        m_bbox.Union( primitive->GetBBox() );

    // enlarge the bounding box a little
    static const auto threshold = 0.001f;
    auto delta = (m_bbox.m_Max - m_bbox.m_Min ) * threshold;
    m_bbox.m_Min -= delta;
    m_bbox.m_Max += delta;
}

bool Accelerator::GetAttenuation( Ray& r , Spectrum& attenuation ) const {
    Intersection intersection;
    if( !GetIntersect( r , &intersection ) )
        return false;

    sAssert( nullptr != intersection.primitive , SPATIAL_ACCELERATOR );

    const Material* material = intersection.primitive->GetMaterial();

    sAssert( nullptr != material , SPATIAL_ACCELERATOR );

    r.m_Ori = intersection.intersect;

    attenuation = material->EvaluateTransparency( intersection );

    return true;
}