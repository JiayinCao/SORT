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

	// Find vectors for two edges sharing vert0
    Vector edge1 = p1 - p0;
    Vector edge2 = p2 - p0;

	// Begin calculating determinant - also used to calculate U parameter
	Vector pvec = Cross( r.m_Dir , edge2 );

    // If determinant is near zero, ray lies in plane of triangle
    float det = Dot( edge1, pvec );

    Vector tvec;
    if( det > 0 )
        tvec = r.m_Ori - p0;
    else
    {
        tvec = p0 - r.m_Ori;
        det = -det;
    }

    if( det < 0.0001f )
        return -1.0f;

    // Calculate U parameter and test bounds
    float u = Dot( tvec, pvec );
    if( u < 0.0f || u > det )
        return -1.0f;

    // Prepare to test V parameter
    Vector qvec = Cross( tvec, edge1 );

    // Calculate V parameter and test bounds
    float v = Dot( r.m_Dir, qvec );
    if( v < 0.0f || u + v > det )
        return -1.0f;

    // Calculate t, scale parameters, ray intersects triangle
    float t = Dot( edge2, qvec );
    float fInvDet = 1.0f / det;
    t *= fInvDet;

    return t;
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
