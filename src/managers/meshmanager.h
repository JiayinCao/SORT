/*
 * filename :	meshmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MESHMANAGER
#define	SORT_MESHMANAGER

// include the headers
#include "sort.h"
#include "utility/singleton.h"
#include "utility/referencecount.h"
#include "utility/enum.h"
#include <vector>
#include <map>
#include "geometry/point.h"
#include "geometry/vector.h"
#include "geometry/transform.h"

// pre-declera class
class MeshLoader;
class TriMesh;

struct VertexIndex
{
	int	posIndex;
	int	norIndex;
	int	texIndex;

	VertexIndex()
	{
		posIndex = -1;
		norIndex = -1;
		texIndex = -1;
	}
	~VertexIndex(){}
};

// the buffer memory for the mesh
class BufferMemory : public ReferenceCount
{
// public data
public:
	// the vertex buffer
	vector<Point>	m_PositionBuffer;
	// the normal buffer
	vector<Vector>	m_NormalBuffer;
	// the texture coordinate buffer
	vector<float>	m_TexCoordBuffer;
	// the index buffer
	vector<VertexIndex>	m_IndexBuffer;
	// the size for three buffers
	unsigned		m_iVBCount , m_iNBCount , m_iTBCount;
	// the number of triangles 
	unsigned		m_iTriNum;
	// the tri mesh
	TriMesh*		m_pPrototype;

	// set default data for the buffer memory
	BufferMemory()
	{
		m_iVBCount = 0;
		m_iNBCount = 0;
		m_iTBCount = 0;
		m_iTriNum = 0;
		m_pPrototype = 0;
	}

	// apply transform
	void ApplyTransform( TriMesh* mesh );
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
