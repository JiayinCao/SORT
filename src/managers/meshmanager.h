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
#include "utility/enum.h"
#include <vector>
#include <map>

// pre-declera class
class Point;
class Vector;
class MeshLoader;
class TriMesh;

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

	// destructor
	~MeshManager(){ _release(); }

	// load the mesh from file
	// para 'str'  : name of the file
	// para 'mesh' : triangle mesh
	// para 'type' : the type of the mesh
	// result      : 'true' if loading is successful
	bool LoadMesh( const string& str , TriMesh* mesh , MESH_TYPE type );

// private field
private:
	// the mesh loaders
	vector<MeshLoader*>	m_MeshLoader;

	// the memory for meshes
	map< string , BufferMemory* > m_Buffers;

// private method
private:
	// default constructor
	MeshManager(){ _init(); }

	// initialize the manager
	void	_init();
	// release the manager
	void	_release();

	// get the mesh loader
	MeshLoader*	_getMeshLoader( MESH_TYPE type ) const;
};

#endif
