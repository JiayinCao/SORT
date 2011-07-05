/*
 * filename :	kdtree.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "kdtree.h"
#include "managers/logmanager.h"

// default constructor
KDTree::KDTree()
{
}

// constructor from a primitive list
KDTree::KDTree( vector<Primitive*>* l ) : Accelerator(l)
{
}
// destructor
KDTree::~KDTree()
{
}

// get the intersection between the ray and the primitive set
bool KDTree::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	//to be modified
	return Intersect( r , m_BBox ) > 0.0f ;
}

// build the acceleration structure
void KDTree::Build()
{
	// get the bounding box for the whole primitive list
	_computeBBox();
}

// output log
void KDTree::OutputLog() const
{
	LOG_HEADER( "Accelerator" );
	LOG<<"KD-Tree is not implemented yet."<<ENDL<<ENDL;
}
