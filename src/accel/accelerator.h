/*
 * filename :	accelerator.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_ACCELERATOR
#define	SORT_ACCELERATOR

// include the header file
#include "sort.h"
#include <vector>
#include "geometry/bbox.h"

// pre-declera classes
class Primitive;
class Intersection;
class Ray;

////////////////////////////////////////////////////////////////////////////////
// definition of accelerator
// Accelerator is a kind of space-partitioning data structure for improving 
// the performance of ray and scene intersection test.
class	Accelerator
{
// public method
public:
	// default constructor
	Accelerator(){ m_primitives = 0; m_pMailBox = 0; }
	// constructor from primitive list
	// para 'l' ; the primitive list
	Accelerator( vector<Primitive*>* l ) : m_primitives(l){}
	// destructor
	virtual ~Accelerator(){ SAFE_DELETE_ARRAY(m_pMailBox); m_primitives = 0;}

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const = 0;

	// build the acceleration structure
	virtual void Build() = 0;

	// output log information
	virtual void OutputLog() const = 0;

	// get bounding box
	const BBox& GetBBox() const { return m_BBox; }

	// set primitive list
	void SetPrimitives( vector<Primitive*>* pri )
	{
		m_primitives = pri;
	}

// protected field
protected:
	// the vector storing primitive list
	vector<Primitive*>* m_primitives;

	// the bounding box of the primitives
	BBox	m_BBox;

	// the mail box for avoiding intersection test with duplicate triangles
	bool*	m_pMailBox;

	// compute the bounding box of the primitives
	void _computeBBox();
};

#endif
