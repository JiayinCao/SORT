/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "sphere.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"
#include "core/log.h"

Point Sphere::Sample_l( const LightSample& ls , const Point& p , Vector& wi , Vector& n , float* pdf ) const{
    sAssertMsg(false, SAMPLING, "N is not filled in Sphere::sample_l");

    const auto center = m_transform( Point( 0.0f , 0.0f , 0.0f ) );
    const auto delta = center - p;
    const auto dir = Normalize( delta );
    Vector wcx , wcy;
    CoordinateSystem( dir , wcx , wcy );

    Matrix m(   wcx.x , dir.x , wcy.x , 0.0f ,
                wcx.y , dir.y , wcy.y , 0.0f ,
                wcx.z , dir.z , wcy.z , 0.0f ,
                0.0f , 0.0f , 0.0f , 1.0f );

    const auto sq_sin_theta = radius * radius / delta.SquaredLength();
    const auto cos_theta = sqrt( std::max( 0.0f , 1.0f - sq_sin_theta ) );

    wi = UniformSampleCone( ls.u , ls.v , cos_theta );
    wi = m(wi);

    if( pdf ) *pdf = UniformConePdf( cos_theta );

    Intersection intersection;
    const auto r = m_transform.invMatrix(Ray( p , wi ));
    if( !GetIntersect( r , &intersection ) )
        intersection.intersect = r( Dot( delta , wi ) );

    return m_transform(intersection.intersect);
}

float Sphere::Pdf( const Point& p ,  const Vector& wi ) const{
    Point center;
    const auto sin_theta_sq = radius * radius / ( p - center ).SquaredLength();
    const auto cos_theta = sqrt( std::max( 0.0f , 1.0f - sin_theta_sq ) );
    return UniformConePdf( cos_theta );
}

float Sphere::SurfaceArea() const{
    return 4 * PI * radius * radius ;
}

bool Sphere::GetIntersect( const Ray& ray , Intersection* intersect ) const{
    const auto r = m_transform.invMatrix( ray );

    const auto _b = 2.0f * ( r.m_Dir.x * r.m_Ori.x + r.m_Dir.y * r.m_Ori.y + r.m_Dir.z * r.m_Ori.z );
    const auto _c = r.m_Ori.x * r.m_Ori.x + r.m_Ori.y * r.m_Ori.y + r.m_Ori.z * r.m_Ori.z - radius * radius;

    auto delta = _b * _b - 4.0f * _c ;
    if( delta < 0.0f )
        return false;
    delta = sqrt( delta );

    const auto min_t = ( -_b - delta ) * 0.5f;
    const auto max_t = ( -_b + delta ) * 0.5f;

    const auto limit = intersect ? intersect->t : FLT_MAX;
    if( min_t > limit || max_t <= 0.0f )
        return false;

    float t;
    if( min_t > 0.0f )
        t = min_t;
    else if( max_t > limit )
        return false;
    else
        t = max_t;

    if( t > r.m_fMax || t < r.m_fMin )
        return false;

    const auto p = r(t);
    if( intersect )
    {
        intersect->t = t;
        Vector n = Normalize(Vector( p.x , p.y , p.z ));
        Vector v0 , v1;
        CoordinateSystem( n , v0 , v1 );
        intersect->intersect = m_transform(p);
        intersect->normal = m_transform.invMatrix.Transpose()(n);
        intersect->gnormal = intersect->normal;
        intersect->tangent = m_transform(v0);
        intersect->view = -r.m_Dir;
    }

    return true;
}

// sample a ray from light
void Sphere::Sample_l( const LightSample& ls , Ray& r , Vector& n , float* pdf ) const
{
    r.m_fMin = 0.0f;
    r.m_fMax = FLT_MAX;
    Vector normalized_dir = UniformSampleSphere( ls.u , ls.v );
    r.m_Ori = radius * normalized_dir;
    r.m_Dir = UniformSampleSphere( sort_canonical() , sort_canonical() );
    if( Dot( r.m_Dir , Vector( r.m_Ori.x , r.m_Ori.y , r.m_Ori.z ) ) < 0.0f )
        r.m_Dir = -r.m_Dir;
    n = m_transform.invMatrix.Transpose()( Vector( normalized_dir.x , normalized_dir.y , normalized_dir.z ) );

    if( pdf ) *pdf = 1.0f / ( 8.0f * PI * PI * radius * radius );
}

// get the bounding box of the primitive
const BBox& Sphere::GetBBox() const
{
    Point center = m_transform( Point( 0.0f , 0.0f , 0.0f ) );

    if( !m_bbox )
    {
        m_bbox = std::make_unique<BBox>();
        Vector vec_r = Vector( radius , radius , radius );
        m_bbox->m_Min = center - vec_r ;
        m_bbox->m_Max = center + vec_r ;
    }

    return *m_bbox;
}
