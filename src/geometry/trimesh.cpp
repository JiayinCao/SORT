/*
 * filename :	trimesh.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "trimesh.h"
#include "sort.h"
#include "point.h"
#include "managers/meshmanager.h"

// default constructor
TriMesh::TriMesh()
{
	// initialize default data
	_init();
}
// destructor
TriMesh::~TriMesh()
{
	// release the memory
	Release();
}

// initialize default data
void TriMesh::_init()
{
	m_pMemory = 0;
}

// release the memory
void TriMesh::Release()
{
}

// load the mesh
bool TriMesh::LoadMesh( const string& str , MESH_TYPE type )
{
	// release the mesh first
	Release();

	// load the mesh
	return MeshManager::GetSingleton().LoadMesh( str , this , type );
}
