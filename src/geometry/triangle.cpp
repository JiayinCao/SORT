/*
 * filename :	triangle.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "triangle.h"
#include "trimesh.h"

// constructor
Triangle::Triangle( const TriMesh* trimesh , const unsigned index ):
Primitive( trimesh , index )
{
}

// check if the triangle is intersected with the ray
bool Triangle::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	return _getIntersect( r , intersect );
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
