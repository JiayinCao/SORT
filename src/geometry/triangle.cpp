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

	float delta = 0.0001f;
	Vector e1 = p1 - p0;
	Vector e2 = p2 - p0;
	Vector s1 = Cross( r.m_Dir , e2 );
	float divisor = Dot( s1 , e1 );
	if( fabs(divisor) < delta )
		return false;
	float invDivisor = 1.0f / divisor;

	Vector d = r.m_Ori - p0;
	float u = Dot( d , s1 ) * invDivisor;
	if( u < -delta || u > 1.0f + delta )
		return false;
	Vector s2 = Cross( d , e1 );
	float v = Dot( r.m_Dir , s2 ) * invDivisor;
	if( v < -delta || u + v > 1.0f + delta )
		return false;
	float t = Dot( e2 , s2 ) * invDivisor;
	if( t < r.m_fMin || t > r.m_fMax )
		return false;

	if( intersect == 0 )
		return t > r.m_fMin && t < r.m_fMax;

	// if t is out of range , return false
	if( t > intersect->t )
		return false;

	// store the intersection
	intersect->intersect = r(t);

	float w = 1 - u - v;

	// store normal if the info is available
	id0 = m_Index[0].norIndex;
	id1 = m_Index[1].norIndex;
	id2 = m_Index[2].norIndex;

	intersect->normal = ( w * mem->m_NormalBuffer[id0] + u * mem->m_NormalBuffer[id1] + v * mem->m_NormalBuffer[id2]).Normalize();
	intersect->tangent = ( w * mem->m_TangentBuffer[id0] + u * mem->m_TangentBuffer[id1] + v * mem->m_TangentBuffer[id2]).Normalize();

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
const BBox& Triangle::GetBBox() const
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