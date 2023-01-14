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

#include "disk.h"
#include "core/samplemethod.h"
#include "sampler/sample.h"
#include "accel/embree.h"

Point Disk::Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const{
    float u , v;
    UniformSampleDisk( ls.u , ls.v , u , v );

    Point lp = m_transform.TransformPoint( Point( u * radius , 0.0f , v * radius ) );
    n = m_transform.TransformVector( Vector( 0 , 1.0f , 0 ) );
    Vector delta = lp - p;
    wi = normalize( delta );

    float d = dot( -wi , n );
    if( pdf ){
        if( d <= 0.0f )
            *pdf = 0.0f;
        else
            *pdf = delta.SquaredLength() / ( SurfaceArea() * d );
    }

    return lp;
}

void Disk::Sample_l( RenderContext& rc, const LightSample& ls , Ray& r , Vector& n , float* pdf ) const{
    float u , v;
    UniformSampleDisk( ls.u , ls.v , u , v );
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    r.m_Ori = m_transform.TransformPoint(Point( u * radius , 0.0f , v * radius ));
    Vector wi = UniformSampleHemisphere(sort_rand<float>(rc) , sort_rand<float>(rc));
    r.m_Dir = m_transform.TransformVector(wi);
    n = m_transform.TransformNormal( DIR_UP );

    if( pdf ) *pdf = 1.0f / ( radius * radius * PI * TWO_PI );
}

float Disk::SurfaceArea() const{
    return PI * radius * radius;
}

bool Disk::GetIntersect( const Ray& r , SurfaceInteraction* intersect ) const{
    const auto ray = m_transform.invMatrix( r );
    if( ray.m_Dir.y == 0.0f )
        return false;

    const auto limit = intersect ? intersect->t : FLT_MAX;
    const auto t = -ray.m_Ori.y / ray.m_Dir.y;
    if( t > limit || t <= ray.m_fMin || t > ray.m_fMax )
        return false;

    const auto p = ray(t);
    const auto sqLength = p.x * p.x + p.z * p.z;
    if( sqLength > radius * radius )
        return false;

    if( intersect ){
        intersect->t = t;
        intersect->intersect = m_transform.TransformPoint( p );
        intersect->normal = m_transform.TransformNormal(DIR_UP);
        intersect->gnormal = intersect->normal;
        intersect->tangent = m_transform.TransformVector(Vector( 0.0f , 0.0f , 1.0f ));
        intersect->view = -r.m_Dir;
    }

    return true;
}

const BBox& Disk::GetBBox() const{
    if( !m_bbox ){
        m_bbox = std::make_unique<BBox>();
        m_bbox->Union( m_transform.TransformPoint( Point( radius , 0.0f , radius ) ) );
        m_bbox->Union( m_transform.TransformPoint( Point( radius , 0.0f , -radius ) ) );
        m_bbox->Union( m_transform.TransformPoint( Point( -radius , 0.0f , radius ) ) );
        m_bbox->Union( m_transform.TransformPoint( Point( -radius , 0.0f , -radius ) ) );
    }

    return *m_bbox;
}

#if INTEL_EMBREE_ENABLED
void Disk::ConvertIntersection(const RTCRayHit& ray_hit, SurfaceInteraction& sinter) const{
    sinter.t = ray_hit.ray.tfar;
    sinter.normal = Vector(ray_hit.hit.Ng_x, ray_hit.hit.Ng_y, ray_hit.hit.Ng_z);
    sinter.gnormal = sinter.normal;
    sinter.tangent = m_transform.TransformVector(Vector(0.0f, 0.0f, 1.0f));

    const auto& ray = ray_hit.ray;
    sinter.intersect = Vector(ray.org_x + ray.dir_x * sinter.t,
                              ray.org_y + ray.dir_y * sinter.t,
                              ray.org_z + ray.dir_z * sinter.t);
    sinter.view = -Vector(ray.dir_x, ray.dir_y, ray.dir_z);
}

EmbreeGeometry* Disk::BuildEmbreeGeometry(RTCDevice device, Embree& embree) const{
    return buildEmbreeGeometry(device, embree, this);
}
#endif