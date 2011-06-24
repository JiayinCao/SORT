/*
 * filename :	instancetri.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "instancetri.h"
#include "transform.h"
#include "intersection.h"
#include "utility/referencecount.h"
#include "managers/meshmanager.h"
#include "trimesh.h"

// constructor from a triangle
InstanceTriangle::InstanceTriangle( const TriMesh* mesh , const unsigned index , Transform* t ):
Primitive( mesh , index ) , transform( t )
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
	Ray ray = transform->invMatrix( r ) ;

	// get the intersection result 
	bool result =  _getIntersect( ray , intersect );

	if( result == false )
		return false;

	// transform the intersection
	intersect->intersect = (*transform)(intersect->intersect);
	intersect->normal = (*transform)(intersect->normal);

	return true;
}

// get the bounding box of the triangle
const BBox& InstanceTriangle::GetBBox()
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		// to be modified
		m_bbox = new BBox();

		const Reference<BufferMemory> mem = m_trimesh->m_pMemory;
		const VertexIndex* index = &(mem->m_IndexBuffer[ 3 * m_id ]);
		int id0 = index[0].posIndex;
		int id1 = index[1].posIndex;
		int id2 = index[2].posIndex;
	
		// get three vertexes
		const Point& p0 = mem->m_PositionBuffer[id0] ;
		const Point& p1 = mem->m_PositionBuffer[id1] ;
		const Point& p2 = mem->m_PositionBuffer[id2] ;

		// transform the point
		
	}

	return *m_bbox;
}
