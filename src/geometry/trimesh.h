/*
 * filename :	trimesh.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TRIMESH
#define	SORT_TRIMESH

#include <vector>
//#include "triangle.h"

class	Vector;
class	Point;

//////////////////////////////////////////////////////////////////////////////////
//	defination of trimesh
class TriMesh
{
// public method
public:
	// default constructor
	TriMesh();
	// destructor
	~TriMesh();

	// release the memory
	void Release();

// private field
public:
	// the vertex buffer
	Point*			m_pVertexBuffer;
	// the normal buffer
	Vector*			m_pNormalBuffer;
	// the index buffer
	unsigned*		m_pIndexBuffer;
	// the size for three buffers
	unsigned		m_iVBCount , m_iNBCount , m_iIBCount;
	// the number of triangles , usually it's equal to the third of 'm_iIBCount'
	unsigned		m_iTriNum;

	// the triangle list
//	std::vector<Triangle>	m_TriList;

// private method
	// initialize default data
	void	_init();
};

#endif
