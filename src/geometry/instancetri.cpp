/*
   FileName:      instancetri.cpp

   Created Time:  2011-08-04 12:49:20

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "instancetri.h"
#include "intersection.h"
#include "trimesh.h"

// constructor from a triangle
InstanceTriangle::InstanceTriangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , Transform* t , Material* mat  , bool emissive ):
Triangle( pid , mesh , index , mat , emissive ) , transform( t )
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
	bool result =  Triangle::GetIntersect( ray , intersect );
	if( result == false )
		return false;

	// transform the intersection
	if( intersect )
	{
		intersect->intersect = (*transform)(intersect->intersect);
		intersect->normal = ((*transform)(intersect->normal)).Normalize();
		intersect->tangent = ((*transform)(intersect->tangent)).Normalize();
	}

	return true;
}

// get the bounding box of the triangle
const BBox& InstanceTriangle::GetBBox() const
{
	// if there is no bounding box , cache it
	if( !m_bbox )
	{
		// to be modified
		m_bbox = new BBox();

		const Reference<BufferMemory> mem = m_trimesh->m_pMemory;
		int id0 = m_Index[0].posIndex;
		int id1 = m_Index[1].posIndex;
		int id2 = m_Index[2].posIndex;
	
		// get three vertexes
		const Point& p0 = (*transform)(mem->m_PositionBuffer[id0]);
		const Point& p1 = (*transform)(mem->m_PositionBuffer[id1]);
		const Point& p2 = (*transform)(mem->m_PositionBuffer[id2]);

		// transform the point
		m_bbox->Union( p0 );
		m_bbox->Union( p1 );
		m_bbox->Union( p2 );
	}

	return *m_bbox;
}
