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

// index for a vertex
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

// trunk (submesh)
// a trunk only contains index information
class Trunk
{
// public method
public:
	// the name for the current trunk
	string	name;
	// index buffer
	vector<VertexIndex>	m_IndexBuffer;
	// the triangle number
	unsigned	m_iTriNum;

	// the name for the trunk
	Trunk( const string& str ) : name(str)
	{ m_iTriNum = 0; }
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
	// the trunk buffer
	vector<Trunk*>	m_TrunkBuffer;
	// the size for three buffers
	unsigned		m_iVBCount , m_iNBCount , m_iTBCount;
	// the number of triangles 
	unsigned		m_iTriNum;
	// the trunk number
	unsigned		m_iTrunkNum;
	// the tri mesh
	TriMesh*		m_pPrototype;
	// the name for the file
	std::string		m_filename;

	// set default data for the buffer memory
	BufferMemory()
	{
		m_iVBCount = 0;
		m_iNBCount = 0;
		m_iTBCount = 0;
		m_iTriNum = 0;
		m_pPrototype = 0;
		m_iTrunkNum = 0;
	}
	~BufferMemory()
	{
		vector<Trunk*>::iterator it = m_TrunkBuffer.begin();
		while( it != m_TrunkBuffer.end() )
		{
			delete *it;
			it++;
		}
	}

	// apply transform
	void ApplyTransform( TriMesh* mesh );

	// calculate buffer number
	void CalculateCount()
	{
		m_iVBCount = m_PositionBuffer.size();
		m_iTBCount = m_TexCoordBuffer.size();
		m_iNBCount = m_NormalBuffer.size();
		m_iTrunkNum = m_TrunkBuffer.size();
		m_iTriNum = 0;
		vector<Trunk*>::iterator it = m_TrunkBuffer.begin();
		while( it != m_TrunkBuffer.end() )
		{
			(*it)->m_iTriNum = (*it)->m_IndexBuffer.size() / 3;
			m_iTriNum += (*it)->m_iTriNum;
			it++;
		}
	}
};

/////////////////////////////////////////////////////////////////////////
// definition of mesh manager
class	MeshManager : public Singleton<MeshManager>
{
// public method
public:
	// destructor
	~MeshManager(){ _release(); }

	// load the mesh from file
	// para 'str'  : name of the file
	// para 'mesh' : triangle mesh
	// result      : 'true' if loading is successful
	bool LoadMesh( const string& str , TriMesh* mesh );

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

	friend class Singleton<MeshManager>;
};

#endif
