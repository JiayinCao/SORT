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
class	Intersection;
class	Bsdf;

//////////////////////////////////////////////////////////////////
//	definition of primitive
class	Primitive
{
// public method
public:
	// constructor from a id
	Primitive( unsigned id ) { m_primitive_id = id; m_bsdf = 0; }
	// destructor
	virtual ~Primitive(){}

	// get the intersection between a ray and a primitive
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const = 0;

	// get the bounding box of the primitive
	virtual const BBox&	GetBBox() = 0;

	// delete the cache
	virtual void ClearBBoxCache()
	{
		m_bbox.Delete();
	}

	// get primitive id
	unsigned GetID() const { return m_primitive_id; }

	// get bsdf
	Bsdf*	GetBsdf() const { return m_bsdf; }

// protected field
protected:
	// bounding box
	SmartPtr<BBox> m_bbox;
	// id for the primitive
	unsigned m_primitive_id;

	// the material for the primitive
	Bsdf*	m_bsdf;
};

#endif
