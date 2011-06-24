/*
 * filename :	primitive.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_PRIMITIVE
#define	SORT_PRIMITIVE

// include header file
#include "utility/smartptr.h"
#include "bbox.h"

// pre-decleration
class Ray;
class Intersection;
class TriMesh;

//////////////////////////////////////////////////////////////////
//	definition of primitive
class	Primitive
{
// public method
public:
	// constructor from a transformation
	Primitive( const TriMesh* mesh , unsigned id ): m_trimesh(mesh) , m_id(id) {}
	// destructor
	~Primitive(){}

	// get the intersection between a ray and a primitive
	virtual bool	GetIntersect( const Ray& r , Intersection* intersect ) const = 0;

	// get the bounding box of the primitive
	virtual const BBox&	GetBBox() = 0;

	// delete the cache
	virtual void ClearBBoxCache()
	{
		m_bbox.Delete();
	}

// protected field
protected:
	// bounding box
	SmartPtr<BBox> m_bbox;

	// the triangle mesh
	const TriMesh*	m_trimesh;
	// the index
	const unsigned	m_id;

	// get the intersection between a ray and a triangle
	bool	_getIntersect( const Ray& r , Intersection* intersect ) const;
};

#endif
