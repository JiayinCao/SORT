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

bool Line::GetIntersect( const Ray& r , Point& p , Intersection* intersect ) const{
	// Transform the ray from world space to local space.
	const auto ray = m_transform.invMatrix( r );

	// Calculating the intersection between the ray and the plane, whose normal is perpidencular to 
	// both of the line and the ray forward direction.
	const auto v0 = m_p0 - m_p1;				// Vector pointing along the line.
	const auto v1 = Cross( r.m_Dir , v0 );	// Vector that is perpidencular to both v0 and ray forward direction.

	// This could happen if the ray forward direction is the same with the line direction.
	if( v1.SquaredLength() <= 0.0f )
		return false;

	// Distance along the ray direction to the intersection, early out if it is negative value.
	const auto t0 = Dot( v1 , m_p0 - r.m_Ori ) / Dot( r.m_Dir , v1 );
	if( t0 <= 0 )
		return false;

	// This is the intersected point.
	const auto p0 = r.m_Ori + t0 * r.m_Dir;

	// Calculate the point on the line that is nearest to the intersected point.
	// Early out if necessary.
	const auto t1 = Dot( -v0 , m_p0 - p0 ) / v0.SquaredLength();
	if( t1 < 0 || t0 > 1.0f )
		return false;
	const auto p1 = lerp( m_p0 , m_p1 , t1 );

	// Checking distance between the two intersected points.
	const auto v2 = p1 - p0;
	const auto w = lerp( m_w0 , m_w1 , t1 );
	if( v2.SquaredLength() > w * w * 0.25f )
		return false;

	// There is an intersection between the ray and the line.
	p = m_p0 + v0 * w;
	if( intersect ){
		intersect->intersect = p0;
		intersect->gnormal = Normalize( v1 );
		intersect->normal = Normalize( v1 );
		intersect->tangent = Normalize( v0 );

		intersect->u = t1;
		const auto neg = Dot( Cross( v2 , v0 ) , v1 ) < 0.0f;
		intersect->v = 0.5f + ( ( neg ? -0.5f : 0.5f ) * v2.Length() / w );
		intersect->t = t0;
	}

	return false;
}

const BBox& Line::GetBBox() const{
	if( !m_bbox ){
		m_bbox = std::make_unique<BBox>();
		m_bbox->Union( m_transform(m_p0) );
		m_bbox->Union( m_transform(m_p1) );
		m_bbox->Expend( std::max( m_w0 , m_w1 ) * 0.5f );
	}
	return *m_bbox;
}

float Line::SurfaceArea() const{
	return m_length * ( m_w0 + m_w1 ) * 0.5f;
}

bool Line::GetIntersect(const BBox& box) const{
	return false;
}