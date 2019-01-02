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

#include "triangle.h"
#include "entity/visual.h"

static inline int MajorAxis( const Vector3f& v ){
	if( abs( v[0] ) > abs( v[1] ) && abs( v[0] ) > abs( v[2] ) ) 
		return 0;
	return abs( v[1] ) > abs( v[2] ) ? 1 : 2;
}
static inline Vector3f Permute( const Vector3f& v , int ax , int ay , int az ){
	return Vector3f( v[ax] , v[ay] , v[az] );
}

// 'Watertight Ray/Triangle Intersection' (Sven Woop, Carsten Benthin, Ingo Wald)
// http://jcgt.org/published/0002/01/05/paper.pdf
bool Triangle::GetIntersect( const Ray& r , Point& p , Intersection* intersect ) const
{
	// get the memory
	// note : reference is not used here because it's not thread-safe
	auto& mem = m_meshVisual->m_memory;
	int id0 = m_index.m_id[0];
	int id1 = m_index.m_id[1];
	int id2 = m_index.m_id[2];

    const MeshVertex& mv0 = mem->m_vertices[id0];
    const MeshVertex& mv1 = mem->m_vertices[id1];
    const MeshVertex& mv2 = mem->m_vertices[id2];

	// get three vertexes
	const Point& op0 = mv0.m_position;
	const Point& op1 = mv1.m_position;
	const Point& op2 = mv2.m_position;

	Point p0 = op0;
	Point p1 = op1;
	Point p2 = op2;

	// transform the vertices from world space to ray coordinate
	// step 0 : translate the vertices with ray origin
	p0 -= r.m_Ori;
	p1 -= r.m_Ori;
	p2 -= r.m_Ori;

	// step 1 : pick the major axis to avoid dividing by zero in the sheering pass.
	//          by picking the major axis, we can also make sure we sheer as little as possible
	const int ay = MajorAxis( r.m_Dir );
	const int az = ( ay + 1 ) % 3;
	const int ax = ( az + 1 ) % 3;
	Vector3f d = Permute( r.m_Dir , ax , ay , az );
	p0 = Permute( p0 , ax , ay , az );
	p1 = Permute( p1 , ax , ay , az );
	p2 = Permute( p2 , ax , ay , az );

	// step 2 : sheer the vertices so that the ray direction points to ( 0 , 1 , 0 )
	const float sx = -d.x / d.y;
	const float sz = -d.z / d.y;
	const float sy = 1.0f / d.y;
	p0.x += sx * p0.y;
	p0.z += sz * p0.y;
	p1.x += sx * p1.y;
	p1.z += sz * p1.y;
	p2.x += sx * p2.y;
	p2.z += sz * p2.y;

	// compute the edge functions
	float e0 = p1.x * p2.z - p1.z * p2.x;
	float e1 = p2.x * p0.z - p2.z * p0.x;
	float e2 = p0.x * p1.z - p0.z * p1.x;

	// fall back to double precision for better accuracy at some performance cost
	if( ( e0 == 0.0f || e1 == 0.0f || e2 == 0.0f ) ){
		e0 = (float)( (double)p1.x * (double)p2.z - (double)p1.z * (double)p2.x );
		e1 = (float)( (double)p2.x * (double)p0.z - (double)p2.z * (double)p0.x );
		e2 = (float)( (double)p0.x * (double)p1.z - (double)p0.z * (double)p1.x );
	}

	if( ( e0 < 0 || e1 < 0 || e2 < 0 ) && ( e0 > 0 || e1 > 0 || e2 > 0 ) )
		return false;
	const float det = e0 + e1 + e2;
	if( det == .0f )
		return false;

	p0.y *= sy;
	p1.y *= sy;
	p2.y *= sy;
	const float invDet = 1.0f / det;
	float t = ( e0 * p0.y + e1 * p1.y + e2 * p2.y ) * invDet;
	if( t <= r.m_fMin || t >= r.m_fMax )
		return false;
	if( intersect == nullptr )
		return true;
	if( t > intersect->t )
		return false;

	const float u = e1 * invDet;
	const float v = e2 * invDet;
	const float w = 1 - u - v;

	// store the intersection
	intersect->intersect = r(t);

    intersect->gnormal = Normalize(Cross( ( op2 - op0 ) , ( op1 - op0 ) ));
	intersect->normal = ( w * mv0.m_normal + u * mv1.m_normal + v * mv2.m_normal).Normalize();
	intersect->tangent = ( w * mv0.m_tangent + u * mv1.m_tangent + v * mv2.m_tangent).Normalize();

    auto uv = w * mv0.m_texCoord + u * mv1.m_texCoord + v * mv2.m_texCoord;
    intersect->u = uv.x;
    intersect->v = uv.y;
	intersect->t = t;

    return t > 0.0f ;
}

// get the bounding box of the triangle
const BBox& Triangle::GetBBox() const
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
        m_bbox = std::make_unique<BBox>();

        auto& mem = m_meshVisual->m_memory;
        int id0 = m_index.m_id[0];
        int id1 = m_index.m_id[1];
        int id2 = m_index.m_id[2];

        const Point& p0 = mem->m_vertices[id0].m_position;
        const Point& p1 = mem->m_vertices[id1].m_position;
        const Point& p2 = mem->m_vertices[id2].m_position;

		m_bbox->Union( p0 );
		m_bbox->Union( p1 );
		m_bbox->Union( p2 );
	}

	return *m_bbox;
}

// get the surface area of the triangle
float Triangle::SurfaceArea() const
{
	auto& mem = m_meshVisual->m_memory;
    int id0 = m_index.m_id[0];
    int id1 = m_index.m_id[1];
    int id2 = m_index.m_id[2];

    const Point& p0 = mem->m_vertices[id0].m_position;
    const Point& p1 = mem->m_vertices[id1].m_position;
    const Point& p2 = mem->m_vertices[id2].m_position;

	Vector e0 = p1 - p0 ;
	Vector e1 = p2 - p0 ;
	Vector t = Cross( e0 , e1 );

	return t.Length() * 0.5f;
}

// Project vertex along specific axis
static inline void Project(const Point* points, int count , const Vector& axis, float& min, float& max)
{
	for( int i = 0; i < count; ++i ){
		float val = Dot( axis , (Vector)points[i] );
		if (val < min) min = val;
		if (val > max) max = val;
	}
}

// 'Fast 3D Triangle-Box Overlap Testing'
// http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf
bool Triangle::GetIntersect(const BBox& box) const{
    auto& mem = m_meshVisual->m_memory;
    int id0 = m_index.m_id[0];
    int id1 = m_index.m_id[1];
    int id2 = m_index.m_id[2];

    const MeshVertex& mv0 = mem->m_vertices[id0];
    const MeshVertex& mv1 = mem->m_vertices[id1];
    const MeshVertex& mv2 = mem->m_vertices[id2];

    Point tri[3] = { mv0.m_position , mv1.m_position , mv2.m_position };

	float triMin , triMax;	// will initialize later
	float boxMin = FLT_MAX, boxMax = -FLT_MAX;

	Vector	boxN[3] = { Point( 1.0f , 0.0f , 0.0f ) , 
						Point( 0.0f , 1.0f , 0.0f ) ,
						Point( 0.0f , 0.0f , 1.0f )};
	
	// Case 1 : try separating axis perpendicular to box surface normal , 3 tests
	// along bounding box face normal directions first
	for( int i = 0 ; i < 3 ; ++i ){
		triMin = FLT_MAX;
		triMax = -FLT_MAX;
		Project( tri , 3 , boxN[i] , triMin , triMax );
		if( triMin > box.m_Max[i] || triMax < box.m_Min[i] )
			return false;
	}

	// Case 2 : try triangle plane , 1 test
	// get triangle normal
	Vector triN = Cross( tri[1] - tri[0] , tri[2] - tri[0] );
	//triN = triN * ( 1.0f / triN.Length() );	// no need to normalize it at all
	float triOffset = Dot( triN , (Vector)tri[0] );
	Point bbp[8] = { box.m_Min , 
		Point( box.m_Min.x , box.m_Min.y , box.m_Max.z ) ,
		Point( box.m_Min.x , box.m_Max.y , box.m_Min.z ),
		Point( box.m_Min.x , box.m_Max.y , box.m_Max.z ),
		Point( box.m_Max.x , box.m_Min.y , box.m_Min.z ),
		Point( box.m_Max.x , box.m_Min.y , box.m_Max.z ),
		Point( box.m_Max.x , box.m_Max.y , box.m_Min.z ),
		box.m_Max };
	Project( bbp , 8 , triN , boxMin , boxMax );
	if( boxMax < triOffset || boxMin > triOffset )
		return false;

	// Case 3 : try cross product planes , 9 tests
	Vector triangleEdges[3] = { tri[0] - tri[1] , tri[1] - tri[2] , tri[2] - tri[0] };
	for( int i = 0 ; i < 3 ; ++i ){
		for( int j = 0 ; j < 3 ; ++j ){
			triMin = boxMin = FLT_MAX;
			triMax = boxMax = -FLT_MAX;
			Vector new_axis = Cross( triangleEdges[i] , boxN[j] );
			Project( bbp , 8 , new_axis , boxMin , boxMax );
			Project( tri , 3 , new_axis , triMin , triMax );

			if( boxMax < triMin || boxMin > triMax )
				return false;
		}
	}

	return true;
}