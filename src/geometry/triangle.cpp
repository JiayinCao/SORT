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
	int id0 = m_Index[0].posIndex;
	int id1 = m_Index[1].posIndex;
	int id2 = m_Index[2].posIndex;

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

    if( det < -0.0000001f )
		return false;

    // Calculate U parameter and test bounds
    float u = Dot( tvec, pvec );
    if( u < -0.0000001f || u > det + 0.0000001f )
		return false;

    // Prepare to test V parameter
    Vector qvec = Cross( tvec, edge1 );

    // Calculate V parameter and test bounds
    float v = Dot( r.m_Dir, qvec );
    if( v < -0.0000001f || u + v > det + 0.0000001f )
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

	float w = 1 - u - v;

	// store normal if the info is available
	if( mem->m_iNBCount > 0 )
	{
		id0 = m_Index[0].norIndex;
		id1 = m_Index[1].norIndex;
		id2 = m_Index[2].norIndex;

		intersect->normal = ( w * mem->m_NormalBuffer[id0] + u * mem->m_NormalBuffer[id1] + v * mem->m_NormalBuffer[id2]).Normalize();
	}

	// store texture coordinate
	if( mem->m_iTBCount > 0 )
	{
		id0 = 2*m_Index[0].texIndex;
		id1 = 2*m_Index[1].texIndex;
		id2 = 2*m_Index[2].texIndex;

		float u0 = mem->m_TexCoordBuffer[id0];
		float u1 = mem->m_TexCoordBuffer[id1];
		float u2 = mem->m_TexCoordBuffer[id2];
		float v0 = mem->m_TexCoordBuffer[id0+1];
		float v1 = mem->m_TexCoordBuffer[id1+1];
		float v2 = mem->m_TexCoordBuffer[id2+1];
		intersect->u = w * u0 + u * u1 + v * u2;
		intersect->v = w * v0 + u * v1 + v * v2;

		float du1 = u0 - u2;
		float du2 = u1 - u2;
		float dv1 = v0 - v2;
		float dv2 = v1 - v2;
		Vector dp1 = p0 - p2;
		Vector dp2 = p1 - p2;

		float determinant = du1 * dv2 - dv1 * du2 ;
		float invdet = 1.0f / determinant;
		intersect->dpdu = ( dv2 * dp1 - dv1 * dp2 ) * invdet;
		intersect->dpdv = ( -du2 * dp1 + du1 * dp2 ) * invdet;
	}else
	{
		intersect->u = 0.0f;
		intersect->v = 0.0f;
	}

	intersect->t = t;
	intersect->primitive = const_cast<Triangle*>(this);

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
		int id0 = m_Index[ 0 ].posIndex;
		int id1 = m_Index[ 1 ].posIndex;
		int id2 = m_Index[ 2 ].posIndex;

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
