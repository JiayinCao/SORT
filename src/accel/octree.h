/*
   FileName:      octree.h

   Created Time:  2016-3-7

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_OCTREE
#define	SORT_OCTREE

#include "accelerator.h"

/////////////////////////////////////////////////////////////////////////////////////
//	definition of OcTree
class OcTree : public Accelerator
{
// public method
public:
	DEFINE_CREATOR( OcTree , "octree" );

	// default construction
	OcTree();
	// destructor
	~OcTree();

	// get the intersection between the ray and the primitive set
	// para 'r' : the ray
	// para 'intersect' : the intersection result
	// result   : 'true' if the ray pirece one of the triangle in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// build the acceleration structure
	virtual void Build();

	// output log information
	void OutputLog() const;
};

#endif