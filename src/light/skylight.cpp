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

#include "skylight.h"

Spectrum SkyLight::sample_l(const Point& ip, const LightSample* ls, Vector& dirToLight, float* distance, 
                            float* pdfw, float* emissionPdf, float* cosAtLight, Visibility& visibility) const {
    // sample a ray
    float _pdfw = 0.0f;
    Vector local_dir, global_dir;
    SampleLocalDirection(*ls, _pdfw, local_dir, global_dir);
    if (_pdfw == 0.0f)
        return 0.0f;
    dirToLight = global_dir;

    if (pdfw)
        *pdfw = _pdfw;

    if (distance)
        *distance = 1e6f;

    if (cosAtLight)
        *cosAtLight = 1.0f;

    if (emissionPdf){
        const BBox& box = m_scene->GetBBox();
        const Vector delta = box.m_Max - box.m_Min;
        *emissionPdf = _pdfw * 4.0f * INV_PI / delta.SquaredLength();
    }

    // setup visibility tester
    const float delta = 0.01f;
    visibility.ray = Ray(ip, dirToLight, 0, delta, FLT_MAX);

    return RadianceFromDirection(local_dir);
}

Spectrum SkyLight::sample_l( RenderContext& rc, const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const{
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    float _pdfw;
    Vector3f local_dir, global_dir;
    SampleLocalDirection(ls, _pdfw, local_dir, global_dir);
    r.m_Dir = -global_dir;

    const BBox& box = m_scene->GetBBox();
    const Point center = ( box.m_Max + box.m_Min ) * 0.5f;
    const Vector delta = box.m_Max - center;
    float world_radius = delta.Length();

    Vector v1 , v2;
    coordinateSystem( -r.m_Dir , v1 , v2 );
    float d1 , d2;
    const float t0 = sort_rand<float>(rc);
    const float t1 = sort_rand<float>(rc);
    UniformSampleDisk( t0 , t1 , d1 , d2 );
    r.m_Ori = center + world_radius * ( v1 * d1 + v2 * d2 ) - r.m_Dir * 2.0f * world_radius;

    const float emissionPdf = _pdfw / ( PI * world_radius * world_radius );
    if( pdfW )
        *pdfW = emissionPdf;
    if( cosAtLight )
        *cosAtLight = 1.0f;
    if( pdfA )
        *pdfA = _pdfw;

    return RadianceFromDirection(local_dir);
}

Spectrum SkyLight::Le(const SurfaceInteraction& intersect, const Vector& wo, float* directPdfA, float* emissionPdf) const {
    const BBox& box = m_scene->GetBBox();
    const Vector delta = box.m_Max - box.m_Min;

    // flipping wo
    const auto flipped_wo = -wo;

    const float directPdf = Pdf(intersect.intersect, flipped_wo);
    const float positionPdf = 4.0f * INV_PI / delta.SquaredLength();

    if (directPdfA)
        *directPdfA = directPdf;
    if (emissionPdf)
        *emissionPdf = directPdf * positionPdf;

    const auto local_dir = m_light2world.GetInversed().TransformVector(flipped_wo);
    return RadianceFromDirection(local_dir);
}