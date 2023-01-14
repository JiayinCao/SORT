/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "pointlight.h"
#include "math/interaction.h"
#include "core/samplemethod.h"
#include "sampler/sample.h"

// sample ray from light
Spectrum PointLight::sample_l(const Point& ip, const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const{
    const auto light_pos = Point( m_light2world.matrix.m[3] , m_light2world.matrix.m[7] , m_light2world.matrix.m[11] );

    // Get light position
    const auto _dirToLight = light_pos - ip;

    // normalize vec
    const auto sqrLen = _dirToLight.SquaredLength();
    const auto len = sqrt(sqrLen);
    dirToLight = _dirToLight / len;

    // setup visibility ray
    const auto delta = 0.01f;
    visibility.ray = Ray( ip , dirToLight , 0 , delta , len );

    // direction pdf from 'intersect' to light source w.r.t solid angle
    if( pdfw )
        *pdfw = sqrLen;

    if( cosAtLight )
        *cosAtLight = 1.0f;

    if( distance )
        *distance = len;

    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
    if( emissionPdf )
        *emissionPdf = UniformSpherePdf();

    return intensity;
}

// sample a ray from light
Spectrum PointLight::sample_l( RenderContext& rc, const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const{
    auto light_pos = Point( m_light2world.matrix.m[3] , m_light2world.matrix.m[7] , m_light2world.matrix.m[11] );

    // sample a new ray
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    r.m_Ori = light_pos;
    r.m_Dir = UniformSampleSphere( ls.u , ls.v );

    // product of pdf of sampling a point w.r.t surface area and a direction w.r.t direction
    if( pdfW )
        *pdfW = UniformSpherePdf();

    // pdf w.r.t surface area
    if( pdfA )
        *pdfA = 1.0f;

    if( cosAtLight )
        *cosAtLight = 1.0f;

    return intensity;
}
