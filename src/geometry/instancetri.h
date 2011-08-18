/*
   FileName:      instancetri.h

   Created Time:  2011-08-04 12:49:23

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_INSTANCETRI
#define	SORT_INSTANCETRI

// include the header
#include "triangle.h"

// pre-declera class
class Transform;

////////////////////////////////////////////////////////////////////////////////
// definition of instance triangle
// note :	an instance triangle is just a copy of a triangle, it doesn't 
//			save the position data of the triangle. when performing intersection
//			test , it first transform the ray from world space to object space
//			and then to the prototype-primitive space, it costs more than
//			original triangle , but saves memory space.
class	InstanceTriangle : public Triangle
{
// public method
public:
	// constructor from a Triangle
	InstanceTriangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , Transform* transform , Material* mat , bool emissive );
	// destructor
	~InstanceTriangle();

	// get the instersection between a ray and a instance triangle
	bool	GetIntersect( const Ray& r , Intersection* intersect ) const;	
	
	// get the bounding box of the triangle
	const BBox&	GetBBox() const;

// private field
private:
	// the transformation of the triangle
	Transform* transform;
};

#endif
