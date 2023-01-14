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

#include "line.h"
#include "math/utils.h"

bool Line::GetIntersect( const Ray& r , SurfaceInteraction* intersect ) const{
    // convert it to line space first
    const auto ray = m_world2Line( r );

    const auto tmp0 = m_w0 + ray.m_Ori.y * ( m_w1 - m_w0 ) / m_length;
    const auto tmp1 = ray.m_Dir.y * ( m_w1 - m_w0 ) / m_length;

    // The 2.0 factor is skipped because it is not needed and will be canceled out.
    const auto a = SQR(ray.m_Dir.x) + SQR(ray.m_Dir.z) - SQR( tmp1 );
    const auto b = ( ( ray.m_Ori.x * ray.m_Dir.x + ray.m_Ori.z * ray.m_Dir.z ) - tmp0 * tmp1 );
    const auto c = SQR(ray.m_Ori.x) + SQR(ray.m_Ori.z) - SQR( tmp0 );

    const auto discriminant = b * b - a * c;
    if( discriminant <= 0 )
        return false;
    const auto sqrt_dist = sqrt( discriminant );

    float t = ( -b - sqrt_dist ) / a;
    auto inter = ray(t);
    if( inter.y > m_length || inter.y < 0.0f ){
        t = ( -b + sqrt_dist ) / a;
        inter = ray(t);
        if( inter.y > m_length || inter.y < 0.0f )
            return false;
    }

    if( t <= r.m_fMin || t >= r.m_fMax )
        return false;

    if(IS_PTR_INVALID(intersect))
        return true;
    if( t >= intersect->t )
        return false;

    if( intersect ){
        intersect->intersect = r(t);

        if( inter.y == m_length ){
            // A corner case where the tip of the line is being intersected.
            intersect->gnormal = normalize( m_world2Line.GetInversed().TransformVector( Vector( 0.0f , 1.0f , 0.0f ) ) );
            intersect->normal = intersect->gnormal;
            intersect->tangent = normalize( m_world2Line.GetInversed().TransformVector( Vector( 1.0f , 0.0f , 0.0f ) ) );
        }else{
            // There could be better way to calculate the normal with smarter math.
            /*const auto w = lerp( m_w0 , m_w1 , inter.y / m_length );
            Point top( m_w1 * inter.x / w , m_length ,  m_w1 * inter.z / w );
            const auto tangent = normalize( top - inter );
            intersect->tangent = m_world2Line.GetInversed()( tangent );

            const auto normal = Vector( inter.x , 0.0f , inter.z );
            const auto biTangent = cross( normal , tangent );
            intersect->normal = normalize( m_world2Line.GetInversed()( cross( tangent , biTangent ) ) );
            intersect->gnormal = intersect->normal;*/

            // This may not be physically correct, but it should be fine for a pixel width line.
            intersect->gnormal = normalize(m_world2Line.GetInversed().TransformVector( Vector( inter.x , 0.0f , inter.z ) ) );
            intersect->normal = intersect->gnormal;
            intersect->tangent = normalize( m_gp1 - m_gp0 );

            intersect->view = -r.m_Dir;
        }

        intersect->u = 1.0f;
        intersect->v = slerp( m_v0 , m_v1 , inter.y / m_length );
        intersect->t = t;
    }
    return true;
}

const BBox& Line::GetBBox() const{
    if( !m_bbox ){
        m_bbox = std::make_unique<BBox>();
        m_bbox->Union( m_gp0 );
        m_bbox->Union( m_gp1 );
        m_bbox->Expend( std::max( m_w0 , m_w1 ) );
    }
    return *m_bbox;
}

float Line::SurfaceArea() const{
    return m_length * ( m_w0 + m_w1 ) * PI;
}

bool Line::GetIntersect(const BBox& box) const{
    return true;
}

void Line::SetTransform( const Transform& transform ){
    m_transform = transform;

    m_gp0 = transform.TransformPoint( m_p0 );
    m_gp1 = transform.TransformPoint( m_p1 );

    auto y = normalize( m_gp1 - m_gp0 );
    Vector x , z;
    coordinateSystem( y , x , z );
    Matrix world2line(  x.x, x.y, x.z, -( x.x * m_gp0.x + x.y * m_gp0.y + x.z * m_gp0.z ),
                        y.x, y.y, y.z, -( y.x * m_gp0.x + y.y * m_gp0.y + y.z * m_gp0.z ),
                        z.x, z.y, z.z, -( z.x * m_gp0.x + z.y * m_gp0.y + z.z * m_gp0.z ),
                        0.0f, 0.0f, 0.0f, 1.0f);
    m_world2Line = FromMatrix( world2line );

    const auto lp0 = world2line.TransformPoint( m_gp0 );
    const auto lp1 = world2line.TransformPoint( m_gp1 );
    m_length = lp1.y - lp0.y;
}