/*
 * filename :	meshmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MESHMANAGER
#define	SORT_MESHMANAGER

// include the headers
#include "utility/singleton.h"
#include "utility/referencecount.h"

// pre-declera class
class Point;
class Vector;

class BufferMemory : public ReferenceCount
{
// public data
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

	// set default data for the buffer memory
	BufferMemory()
	{
		m_pVertexBuffer = 0;
		m_pNormalBuffer = 0;
		m_pIndexBuffer = 0;
		m_iVBCount = 0;
		m_iNBCount = 0;
		m_iTriNum = 0;
	}
};

/////////////////////////////////////////////////////////////////////////
// definition of mesh manager
class	MeshManager : public Singleton<MeshManager>
{
// public method
public:
	// create mesh manager
	static void CreateMeshManager() { new MeshManager(); }

// private field
private:

	// default constructor
	MeshManager(){}
};

#endif
