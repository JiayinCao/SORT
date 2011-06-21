/*
 * filename :	primitive.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_PRIMITIVE
#define	SORT_PRIMITIVE

#include "../utility/smartptr.h"
#include "bbox.h"

// pre-decleration
class Ray;

//////////////////////////////////////////////////////////////////
//	defination of primitive
class	Primitive
{
// public method
public:
	// default constructor
	Primitive(){}
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
};

#endif
