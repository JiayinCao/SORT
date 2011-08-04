/*
   FileName:      triangle.h

   Created Time:  2011-08-04 12:51:10

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_TRIANGLE
#define	SORT_TRIANGLE

#include "primitive.h"

// pre-decleration
class TriMesh;
struct VertexIndex;

//////////////////////////////////////////////////////////////////////////////////
//	definition of triangle
//	note: triangle is the only primitive supported by the system.
class	Triangle : public Primitive
{
// public method
public:
	// constructor
	// para 'pid'     : primitive id
	// para 'trimesh' : the triangle mesh it belongs to
	// para 'index'   : the index buffer
	Triangle( unsigned pid , const TriMesh* mesh , const VertexIndex* index , Material* mat ):
		Primitive(pid,mat) , m_trimesh(mesh) , m_Index(index) {}
	// destructor
	~Triangle(){}

	// check if the triangle is intersected with the ray
	// para 'r' : the ray to check
	// para 'intersect' : the result storing the intersection information
	//					  the intersection is an optimized versiion
	// result   : positive value if intersect
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	// get the bounding box of the triangle
	virtual const BBox&	GetBBox() const;

// protected filed
protected:
	// the triangle mesh
	const TriMesh*		m_trimesh;
	// the index
	const VertexIndex*	m_Index;
};

#endif
