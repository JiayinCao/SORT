/*
 * filename :	primitive.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_PRIMITIVE
#define	SORT_PRIMITIVE

#include "utility/smartptr.h"
#include "bbox.h"

// pre-decleration
class Ray;
// the transformation of the primitive
class Transform;

//////////////////////////////////////////////////////////////////
//	definition of primitive
class	Primitive
{
// public method
public:
	// constructor from a transformation
	Primitive( Transform* transform ): m_pTransform(transform) {}
	// destructor
	~Primitive(){}

	// get the intersection between a ray and a primitive
	virtual float	GetIntersect( const Ray& r ) const = 0;

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

	// the transformation
	Transform*	m_pTransform;
};

#endif
