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

#include "quad.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"

Point Quad::Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const{
    const float halfx = sizeX * 0.5f;
    const float halfy = sizeY * 0.5f;

    float u = 2 * ls.u - 1.0f;
    float v = 2 * ls.v - 1.0f;
    Point lp = m_transform.TransformPoint( Point( halfx * u , 0.0f , halfy * v ) );
    n = m_transform.TransformVector( Vector( 0 , 1 , 0 ) );
    Vector delta = lp - p;
    wi = normalize( delta );

    float d = dot( -wi , n );
    if( pdf )
    {
        if( d <= 0 )
            *pdf = 0.0f;
        else
            *pdf = delta.SquaredLength() / ( SurfaceArea() * d );
    }

    return lp;
}

void Quad::Sample_l( RenderContext& rc, const LightSample& ls , Ray& r , Vector& n , float* pdf ) const{
    const auto halfx = sizeX * 0.5f;
    const auto halfy = sizeY * 0.5f;

    const auto u = 2 * ls.u - 1.0f;
    const auto v = 2 * ls.v - 1.0f;
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    r.m_Ori = m_transform.TransformPoint( Point( halfx * u , 0.0f , halfy * v ) );
    r.m_Dir = m_transform.TransformVector( UniformSampleHemisphere( sort_rand<float>(rc) , sort_rand<float>(rc) ) );
    n = m_transform.TransformNormal( DIR_UP );

    if( pdf )
        *pdf = UniformHemispherePdf() / SurfaceArea();
}

float Quad::SurfaceArea() const{
    return sizeX * sizeY;
}

bool Quad::GetIntersect( const Ray& r , SurfaceInteraction* intersect ) const{
    const auto ray = m_transform.invMatrix( r );
    if( ray.m_Dir.y == 0.0f )
        return false;

    const auto limit = intersect ? intersect->t : FLT_MAX;
    const auto t = -ray.m_Ori.y / ray.m_Dir.y;
    if( t > limit || t <= ray.m_fMin || t > ray.m_fMax )
        return false;

    const auto p = ray(t);
    const auto halfx = sizeX * 0.5f;
    const auto halfy = sizeY * 0.5f;
    if( p.x > halfx || p.x < -halfx )
        return false;
    if( p.z > halfy || p.z < -halfy )
        return false;

    if( intersect ){
        intersect->t = t;
        intersect->intersect = r(t);
        intersect->normal = m_transform.TransformNormal(DIR_UP);
        intersect->gnormal = intersect->normal;
        intersect->tangent = m_transform.TransformVector(Vector( 0.0f , 0.0f , 1.0f ));
        intersect->view = -r.m_Dir;
    }

    return true;
}

const BBox& Quad::GetBBox() const{
    const auto halfx = sizeX * 0.5f;
    const auto halfy = sizeY * 0.5f;
    if( !m_bbox ){
        m_bbox = std::make_unique<BBox>();
        m_bbox->Union( m_transform.TransformPoint( Point( halfx , 0.0f , halfy ) ) );
        m_bbox->Union( m_transform.TransformPoint( Point( halfx , 0.0f , -halfy ) ) );
        m_bbox->Union( m_transform.TransformPoint( Point( -halfx , 0.0f , halfy ) ) );
        m_bbox->Union( m_transform.TransformPoint( Point( -halfx , 0.0f , -halfy ) ) );
    }

    return *m_bbox;
}
