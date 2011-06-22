/*
 * filename :	meshloader.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MESHLOADER
#define	SORT_MESHLOADER

// include the header
#include "sort.h"
#include "utility/enum.h"

// pre-decleration class
class	BufferMemory;

///////////////////////////////////////////////////////////////////////
//	definition of meshloader
class	MeshLoader
{
// public method
public:
	// default constructor
	MeshLoader(){ m_MeshType = MT_END; }
	// destructor
	~MeshLoader(){}

	// load mesh from file
	// para 'str' : name of the file
	// para 'mem' : the memory to store
	// result     : true if loading is successful
	virtual bool LoadMesh( const string& str , BufferMemory* mem ) = 0;

	// get the type of the mesh loader
	MESH_TYPE GetMT() const { return m_MeshType; }

// protected data
protected:
	// the type for the mesh
	MESH_TYPE	m_MeshType;
};

#endif
