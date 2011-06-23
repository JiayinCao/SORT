/*
 * filename :	triangle.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TRIANGLE
#define	SORT_TRIANGLE

#include "primitive.h"

// pre-decleration
class	TriMesh;
class	Point;
class	Ray;

//////////////////////////////////////////////////////////////////////////////////
//	definition of triangle
//	note: triangle is the only primitive supported by the system.
class	Triangle : public Primitive
{
// public method
public:
	// constructor
	// para 'trimesh' : the triangle mesh it belongs to
	// para 'index'   : the index buffer
	// para 'vb'      : the vertex buffer
	Triangle( const TriMesh* trimesh , const unsigned index , Transform* transform );
	// destructor
	~Triangle(){}

	// check if the triangle is intersected with the ray
	// para 'r' : the ray to check
	// result   : positive value if intersect
	float	GetIntersect( const Ray& r ) const;

	// get the bounding box of the triangle
	const BBox&	GetBBox();

// private field
private:
	// the triangle mesh
	const TriMesh*	m_trimesh;
	// the index
	const unsigned	m_id;
};

#endif
