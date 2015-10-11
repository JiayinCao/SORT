/*
   FileName:      meshmanager.h

   Created Time:  2011-08-04 12:47:40

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MESHMANAGER
#define	SORT_MESHMANAGER

// include the headers
#include "sort.h"
#include "utility/singleton.h"
#include "utility/referencecount.h"
#include "utility/enum.h"
#include "utility/define.h"
#include <vector>
#include <map>
#include "math/point.h"
#include "math/vector3.h"
#include "math/transform.h"
#include "material/material.h"

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
	// the material
	Reference<Material>	m_mat;

	// constructor
	// para 'str' : name for the trunk
	Trunk( const string& str ) : name(str)
	{ m_iTriNum = 0; m_mat = 0; }
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
	// the tagent buffer
	vector<Vector>	m_TangentBuffer;
	// the texture coordinate buffer
	vector<float>	m_TexCoordBuffer;
	// the trunk buffer
	vector<Trunk*>	m_TrunkBuffer;
	// the size for three buffers
	unsigned		m_iVBCount , m_iNBCount , m_iTeBcount , m_iTBCount;
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
		m_iTeBcount = 0;
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
		m_iTeBcount = m_TangentBuffer.size();
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

	// generate normal for the triangle mesh
	void	GenSmoothNormal();
	// generate tagent for the triangle mesh
	void	GenSmoothTagent();
	// generate texture coordinate
	void	GenTexCoord();

// private method
private:
	void	_genFlatNormal();
	Vector	_genTagentForTri( const Trunk* trunk , unsigned k  ) const;
};

/////////////////////////////////////////////////////////////////////////
//	definition of mesh manager
//	desc :	A mesh manager manages all of the geometry data used in the 
//			rendering system. The meshes are not located here. Two meshes
//			will share the same geometry memory if they are loaded from
//			from the same model file, which means intancing is supported
//			in the current system.
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
