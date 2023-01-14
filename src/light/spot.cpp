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

#include "spot.h"
#include "math/interaction.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"

// sample ray from light
Spectrum SpotLight::sample_l(const Point& ip, const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const{
    const auto light_dir = Vector3f( m_light2world.matrix.m[1] , m_light2world.matrix.m[5] , m_light2world.matrix.m[9] );
    const auto light_pos = Point( m_light2world.matrix.m[3] , m_light2world.matrix.m[7] , m_light2world.matrix.m[11] );

    // direction to light
    const Vector _dirToLight = light_pos - ip;

    // normalize vec
    const float sqrLen = _dirToLight.SquaredLength();
    const float len = sqrt(sqrLen);
    dirToLight = _dirToLight / len;

    // direction pdf from 'intersect' to light source w.r.t solid angle
    if( pdfw )
        *pdfw = sqrLen;

    // emission pdf from light source w.r.t solid angle
    if( emissionPdf )
        *emissionPdf = UniformConePdf( cos_total_range );

    if( cosAtLight )
        *cosAtLight = 1.0f;

    if( distance )
        *distance = len;

    // update visility
    const float delta = 0.01f;
    visibility.ray = Ray( ip , dirToLight , 0 , delta , len );

    const float falloff = satDot( dirToLight , -light_dir );
    if( falloff <= cos_total_range )
        return 0.0f;
    if( falloff >= cos_falloff_start )
        return intensity ;
    const float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
    if( d == 0.0f )
        return 0.0f;

    return intensity * d * d;
}

// sample a ray from light
Spectrum SpotLight::sample_l( RenderContext& rc, const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const{
    const auto light_dir = Vector3f( m_light2world.matrix.m[1] , m_light2world.matrix.m[5] , m_light2world.matrix.m[9] );
    const auto light_pos = Point( m_light2world.matrix.m[3] , m_light2world.matrix.m[7] , m_light2world.matrix.m[11] );

    // udpate ray
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    r.m_Ori = light_pos;

    // sample a light direction
    const Vector local_dir = UniformSampleCone( ls.u , ls.v , cos_total_range );
    r.m_Dir = m_light2world.matrix.TransformVector( local_dir );

    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
    if( pdfW )
    {
        *pdfW = UniformConePdf( cos_total_range );
        sAssert( *pdfW != 0.0f , LIGHT );
    }
    // pdf w.r.t surface area
    if( pdfA )
        *pdfA = 1.0f;
    if( cosAtLight )
        *cosAtLight = 1.0f;

    const float falloff = satDot( r.m_Dir , light_dir );
    if( falloff <= cos_total_range )
        return 0.0f;
    if( falloff >= cos_falloff_start )
        return intensity;
    const float d = ( falloff - cos_total_range ) / ( cos_falloff_start - cos_total_range );
    if( d == 0.0f )
        return 0.0f;

    return intensity * d * d;
}