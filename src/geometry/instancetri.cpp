/*
 * filename :	instancetri.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "instancetri.h"
#include "triangle.h"
#include "geometry/transform.h"
#include "geometry/matrix.h"
#include "utility/referencecount.h"
#include "managers/meshmanager.h"

// constructor from a triangle
InstanceTriangle::InstanceTriangle( const TriMesh* mesh , const unsigned index , Transform* transform ):
Primitive( transform )
{
}

// destructor
InstanceTriangle::~InstanceTriangle()
{
}

// get the intersection
bool	InstanceTriangle::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	// transform the ray
	Ray ray = Inverse(*m_pTransform) * r ;
/*
	// get the memory
	Reference<BufferMemory> mem = m_pMemory;
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
	Vector pvec = Cross( ray.m_Dir , edge2 );

    // If determinant is near zero, ray lies in plane of triangle
    float det = Dot( edge1, pvec );

    Vector tvec;
    if( det > 0 )
        tvec = ray.m_Ori - p0;
    else
    {
        tvec = p0 - ray.m_Ori;
        det = -det;
    }

    if( det < 0.0001f )
	{
		intersect->t = -1.0f;	
		return false;
	}

    // Calculate U parameter and test bounds
    float u = Dot( tvec, pvec );
    if( u < 0.0f || u > det )
	{
		intersect->t = -1.0f;	
		return false;
	}

    // Prepare to test V parameter
    Vector qvec = Cross( tvec, edge1 );

    // Calculate V parameter and test bounds
    float v = Dot( ray.m_Dir, qvec );
    if( v < 0.0f || u + v > det )
    {
		intersect->t = -1.0f;	
		return false;
	}

    // Calculate t, scale parameters, ray intersects triangle
    float t = Dot( edge2, qvec );
    float fInvDet = 1.0f / det;
    t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;

	// store the intersection
	intersect->intersect = u * p0 + v * p1 + ( 1 - u - v ) * p2;

	// store normal if the info is available
	if( mem->m_iNBCount >= 0 )
	{
		id0 = index[0].norIndex;
		id1 = index[1].norIndex;
		id2 = index[2].norIndex;

		intersect->normal = ( 1 - v - u ) * mem->m_NormalBuffer[id0] + u * mem->m_NormalBuffer[id1] + v * mem->m_NormalBuffer[id2];
	}

	// store texture coordinate
	if( mem->m_iTBCount > 0 )
	{
		id0 = index[0].texIndex;
		id1 = index[1].texIndex;
		id2 = index[2].texIndex;

		intersect->u = ( 1 - v - u ) * mem->m_TexCoordBuffer[2*id0] + u * mem->m_TexCoordBuffer[2*id1] + v * mem->m_TexCoordBuffer[2*id2];
		intersect->v = ( 1 - v - u ) * mem->m_TexCoordBuffer[2*id0] + u * mem->m_TexCoordBuffer[2*id1+1] + v * mem->m_TexCoordBuffer[2*id2+1];
	}

	intersect->t = t;

    return t > 0.0f ;
*/
	return false;
}

// get the bounding box of the triangle
const BBox& InstanceTriangle::GetBBox()
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		// to be modified
		m_bbox = new BBox();
	}

	return *m_bbox;
}
