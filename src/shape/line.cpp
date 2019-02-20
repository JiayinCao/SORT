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

#include "line.h"

#define SQR(x)		((x)*(x))

bool Line::GetIntersect( const Ray& r , Intersection* intersect ) const{
	// convert it to line space first
	const auto ray = m_world2Line( r );

	const auto tmp0 = m_w0 + ray.m_Ori.y * ( m_w1 - m_w0 ) / m_length;
	const auto tmp1 = ray.m_Dir.y * ( m_w1 - m_w0 ) / m_length;

#if 1
	const auto a = SQR(ray.m_Dir.x) + SQR(ray.m_Dir.z) - SQR( tmp1 );
	const auto b = 2.0f * ( ( ray.m_Ori.x * ray.m_Dir.x + ray.m_Ori.z * ray.m_Dir.z ) - tmp0 * tmp1 );
	const auto c = SQR(ray.m_Ori.x) + SQR(ray.m_Ori.z) - SQR( tmp0 );
#else
	const auto a = SQR(ray.m_Dir.x) + SQR(ray.m_Dir.z);
	const auto b = 2.0f * ( ray.m_Ori.x * ray.m_Dir.x + ray.m_Ori.z * ray.m_Dir.z ) ;
	const auto c = SQR(ray.m_Ori.x) + SQR(ray.m_Ori.z) - SQR( m_w0 );
#endif

	const auto discriminant = b * b - 4.0f * a * c;
	if( discriminant <= 0 )
		return false;
	const auto sqrtDisc = sqrt( discriminant );
	
	float t = ( -b - sqrtDisc ) / ( 2.0f * a );
	auto inter = ray(t);
	if( inter.y > m_length || inter.y < 0.0f ){
		t = ( -b + sqrtDisc ) / ( 2.0f * a );
		inter = ray(t);
		if( inter.y > m_length || inter.y < 0.0f )
			return false;
	}

	if( intersect == nullptr )
		return true;
	if( t >= intersect->t )
		return false;

	// to be removed after figuring out what is wrong in this algorithm.
	const auto delta = 0.01f;
	if( intersect ){
		intersect->intersect = r(t - delta);
		intersect->gnormal = m_world2Line.GetInversed()(Normalize( Vector( inter.x , 0.0f , inter.z ) ));
		intersect->normal = intersect->gnormal;
		intersect->tangent = m_world2Line.GetInversed()(Vector( 0.0f , 1.0f , 0.0f ));

		intersect->u = 0.0f;
		intersect->v = lerp( m_v0 , m_v1 , inter.y / m_length );
		intersect->t = t - delta;
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
	// to be fixed
	return m_length * ( m_w0 + m_w1 ) * 0.5f;
}

bool Line::GetIntersect(const BBox& box) const{
	return true;
}

void Line::SetTransform( const Transform& transform ){
	m_transform = transform; 

	m_gp0 = transform( m_p0 );
	m_gp1 = transform( m_p1 );

	auto y = Normalize( m_gp1 - m_gp0 );
	Vector x , z;
	CoordinateSystem( y , x , z );
	Matrix world2line(	x.x, x.y, x.z, -( x.x * m_gp0.x + x.y * m_gp0.y + x.z * m_gp0.z ),
						y.x, y.y, y.z, -( y.x * m_gp0.x + y.y * m_gp0.y + y.z * m_gp0.z ),
						z.x, z.y, z.z, -( z.x * m_gp0.x + z.y * m_gp0.y + z.z * m_gp0.z ),
						0.0f, 0.0f, 0.0f, 1.0f);
	m_world2Line = FromMatrix( world2line );

	const auto lp0 = world2line( m_gp0 );
	const auto lp1 = world2line( m_gp1 );
	m_length = lp1.y - lp0.y;
}