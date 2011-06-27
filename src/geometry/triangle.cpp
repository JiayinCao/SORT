/*
 * filename :	triangle.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "triangle.h"
#include "trimesh.h"
#include "intersection.h"

// check if the triangle is intersected with the ray
bool Triangle::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	// get the memory
	Reference<BufferMemory> mem = m_trimesh->m_pMemory;
	VertexIndex* index = &(mem->m_IndexBuffer[ 3 * m_id ]);
	int id0 = index[0].posIndex;
	int id1 = index[1].posIndex;
	int id2 = index[2].posIndex;

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

    if( det < 0.000000001f )
		return false;

    // Calculate U parameter and test bounds
    float u = Dot( tvec, pvec );
    if( u < 0.0f || u > det )
		return false;

    // Prepare to test V parameter
    Vector qvec = Cross( tvec, edge1 );

    // Calculate V parameter and test bounds
    float v = Dot( r.m_Dir, qvec );
    if( v < 0.0f || u + v > det )
		return false;

    // Calculate t, scale parameters, ray intersects triangle
    float t = Dot( edge2, qvec );
    float fInvDet = 1.0f / det;
    t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;

	// if t is out of range , return false
	if( t > intersect->t || t < 0.0f || t > r.m_fMax )
		return false;

	// store the intersection
	intersect->intersect = r(t);

	// store normal if the info is available
	if( mem->m_iNBCount > 0 )
	{
		id0 = index[0].norIndex;
		id1 = index[1].norIndex;
		id2 = index[2].norIndex;

		intersect->normal = (( 1 - v - u ) * mem->m_NormalBuffer[id0] + u * mem->m_NormalBuffer[id1] + v * mem->m_NormalBuffer[id2]).Normalize();
	}

	// store texture coordinate
	if( mem->m_iTBCount > 0 )
	{
		id0 = index[0].texIndex;
		id1 = index[1].texIndex;
		id2 = index[2].texIndex;

		intersect->u = ( 1 - v - u ) * mem->m_TexCoordBuffer[2*id0] + u * mem->m_TexCoordBuffer[2*id1] + v * mem->m_TexCoordBuffer[2*id2];
		intersect->v = ( 1 - v - u ) * mem->m_TexCoordBuffer[2*id0+1] + u * mem->m_TexCoordBuffer[2*id1+1] + v * mem->m_TexCoordBuffer[2*id2+1];
	}

	intersect->t = t;
	intersect->u = u;
	intersect->v = v;

    return t > 0.0f ;
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

		m_bbox->Union( p0 );
		m_bbox->Union( p1 );
		m_bbox->Union( p2 );
	}

	return *m_bbox;
}
