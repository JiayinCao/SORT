/*
 * filename :	instancetri.h
 *
 * programmer :	Cao Jiayin
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
	InstanceTriangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , Transform* transform , Material* mat );
	// destructor
	~InstanceTriangle();

	// get the instersection between a ray and a instance triangle
	bool	GetIntersect( const Ray& r , Intersection* intersect ) const;	
	
	// get the bounding box of the triangle
	const BBox&	GetBBox();

// private field
private:
	// the transformation of the triangle
	Transform* transform;
};

#endif
