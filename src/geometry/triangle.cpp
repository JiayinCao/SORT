/*
 * filename :	triangle.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "triangle.h"
#include "trimesh.h"
#include "point.h"
#include "vector.h"
#include "ray.h"
#include "transform.h"

// constructor
Triangle::Triangle( const TriMesh* trimesh , const unsigned index , Transform* transform):
Primitive(transform), m_trimesh( trimesh ) , m_id( index )
{
}

// check if the triangle is intersected with the ray
float Triangle::GetIntersect( const Ray& r ) const
{
	// get the memory
	Reference<BufferMemory> mem = m_trimesh->m_pMemory;
	int id0 = mem->m_IndexBuffer[ 3 * m_id ].posIndex;
	int id1 = mem->m_IndexBuffer[ 3 * m_id + 1 ].posIndex;
	int id2 = mem->m_IndexBuffer[ 3 * m_id + 2 ].posIndex;

	// get three vertexes
	const Point& p0 = mem->m_PositionBuffer[id0] ;
	const Point& p1 = mem->m_PositionBuffer[id1] ;
	const Point& p2 = mem->m_PositionBuffer[id2] ;

	// get the vector
	Vector v0 = p1 - p0;
	Vector v1 = p2 - p1;
	Vector v2 = p0 - p2;

	// get the normal of the plane
	Vector n = Cross( v1 , v2 );

	// get the intersected point
	float t = Dot( n , p0 - r.m_Ori ) / Dot( n , r.m_Dir );
	if( t < 0.0f )
		return t;
	Point p = r(t);

	Vector _v0 = p - p0;
	Vector _v1 = p - p1;
	Vector _v2 = p - p2;

	Vector r0 = Cross( _v0 , v0 );
	Vector r1 = Cross( _v1 , v1 );
	Vector r2 = Cross( _v2 , v2 );

	if( Dot( r0 , r1 ) > -0.0000000001f && Dot( r0 , r2 ) > -0.00000000001f )
		return t;

	// the ray doesn't cross the triangle
	return -1.0f;
}

// get the bounding box of the triangle
const BBox& Triangle::GetBBox()
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		m_bbox = new BBox();

		// get the memory
		Reference<BufferMemory> mem = m_trimesh->m_pMemory;
		int id0 = mem->m_IndexBuffer[ 3 * m_id ].posIndex;
		int id1 = mem->m_IndexBuffer[ 3 * m_id + 1 ].posIndex;
		int id2 = mem->m_IndexBuffer[ 3 * m_id + 2 ].posIndex;

		// get three vertexes
		const Point& p0 = mem->m_PositionBuffer[id0] ;
		const Point& p1 = mem->m_PositionBuffer[id1] ;
		const Point& p2 = mem->m_PositionBuffer[id2] ;

		Union( *m_bbox , p0 );
		Union( *m_bbox , p1 );
		Union( *m_bbox , p2 );
	}

	return *m_bbox;
}
