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

#include "skylight.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"

Spectrum SkyLight::sample_l( const SurfaceInteraction& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const{
    // sample a ray
    float _pdfw = 0.0f;
    const Vector localDir = sky.sample_v( ls->u , ls->v , &_pdfw , 0 );
    if( _pdfw == 0.0f )
        return 0.0f;
    dirToLight = m_light2world.TransformVector(localDir);

    if( pdfw )
        *pdfw = _pdfw;

    if( distance )
        *distance = 1e6f;

    if( cosAtLight )
        *cosAtLight = 1.0f;

    if( emissionPdf )
    {
        const BBox& box = m_scene->GetBBox();
        const Vector delta = box.m_Max - box.m_Min;
        *emissionPdf = _pdfw * 4.0f * INV_PI / delta.SquaredLength();
    }

    // setup visibility tester
    const float delta = 0.01f;
    visibility.ray = Ray( intersect.intersect , dirToLight , 0 , delta , FLT_MAX );

    return sky.Evaluate( localDir ) * intensity;
}

Spectrum SkyLight::Le( const SurfaceInteraction& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const{
    const BBox& box = m_scene->GetBBox();
    const Vector delta = box.m_Max - box.m_Min;

    const float directPdf = sky.Pdf( m_light2world.GetInversed().TransformVector(-wo) );
    const float positionPdf = 4.0f * INV_PI / delta.SquaredLength();

    if( directPdfA )
        *directPdfA = directPdf;
    if( emissionPdf )
        *emissionPdf = directPdf * positionPdf;

    return sky.Evaluate( m_light2world.GetInversed().TransformVector(-wo) ) * intensity;
}

Spectrum SkyLight::sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const{
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    float _pdfw;
    auto localDir = -sky.sample_v( ls.u , ls.v , &_pdfw , 0 );
    r.m_Dir = m_light2world.TransformVector(localDir);

    const BBox& box = m_scene->GetBBox();
    const Point center = ( box.m_Max + box.m_Min ) * 0.5f;
    const Vector delta = box.m_Max - center;
    float world_radius = delta.Length();

    Vector v1 , v2;
    coordinateSystem( -r.m_Dir , v1 , v2 );
    float d1 , d2;
    const float t0 = sort_canonical();
    const float t1 = sort_canonical();
    UniformSampleDisk( t0 , t1 , d1 , d2 );
    r.m_Ori = center + world_radius * ( v1 * d1 + v2 * d2 ) - r.m_Dir * 2.0f * world_radius;

    const float emissionPdf = _pdfw / ( PI * world_radius * world_radius );
    if( pdfW )
        *pdfW = emissionPdf;
    if( cosAtLight )
        *cosAtLight = 1.0f;
    if( pdfA )
        *pdfA = _pdfw;

    return sky.Evaluate( -localDir ) * intensity;
}

Spectrum SkyLight::Power() const{
    sAssert( m_scene != nullptr , LIGHT );
    const BBox box = m_scene->GetBBox();
    const float radius = (box.m_Max - box.m_Min).Length() * 0.5f;

    return radius * radius * PI * sky.GetAverage() * intensity;
}

bool SkyLight::Le( const Ray& ray , SurfaceInteraction* intersect , Spectrum& radiance ) const{
    if( intersect && intersect->t != FLT_MAX )
        return false;

    radiance = sky.Evaluate( m_light2world.GetInversed().TransformVector(ray.m_Dir) ) * intensity;
    return true;
}

float SkyLight::Pdf( const Point& p , const Vector& wi ) const{
    return sky.Pdf( m_light2world.GetInversed().TransformVector(wi) );
}
