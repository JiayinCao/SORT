/*
 * filename :	triangle.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TRIANGLE
#define	SORT_TRIANGLE

// pre-decleration
class	TriMesh;
class	Point;
class	Ray;

//////////////////////////////////////////////////////////////////////////////////
//	defination of triangle
//	note: triangle is the only primitive supported by the system.
class	Triangle
{
// public method
public:
	// constructor
	// para 'trimesh' : the triangle mesh it belongs to
	// para 'index'   : the index buffer
	// para 'vb'      : the vertex buffer
	Triangle( const TriMesh* trimesh , const Point* vb , const unsigned* index );
	// destructor
	~Triangle(){}

	// check if the triangle is intersected with the ray
	// para 'r' : the ray to check
	// result   : positive value if intersect
	float	Intersect( const Ray& r ) const;

// private field
private:
	// the triangle mesh
	const TriMesh*	m_trimesh;
	// the vertex buffer
	const Point*	m_vb;
	// the index
	const unsigned*	m_ib;
};

#endif