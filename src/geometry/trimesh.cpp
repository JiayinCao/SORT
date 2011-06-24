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
#include "geometry/triangle.h"
#include "geometry/instancetri.h"

// default constructor
TriMesh::TriMesh()
{
	// initialize default data
	_init();
}
// destructor
TriMesh::~TriMesh()
{
}

// initialize default data
void TriMesh::_init()
{
	m_pMemory = 0;
}

// load the mesh
bool TriMesh::LoadMesh( const string& str , Transform& transform , MESH_TYPE type )
{
	// set the tranformation
	m_Transform = transform;

	// load the mesh
	return MeshManager::GetSingleton().LoadMesh( str , this , type );
}

// fill buffer into vector
void TriMesh::FillTriBuf( vector<Primitive*>& vec )
{
	if( m_bInstanced == false )
	{
		// generate the triangles
		unsigned triNum = m_pMemory->m_iTriNum;
		for( int i = 0 ; i < (int)triNum ; i++ )
			vec.push_back( new Triangle( this , i , &m_Transform ) );
	}else
	{
		// create instance triangle
		unsigned triNum = m_pMemory->m_iTriNum;
		for( int i = 0 ; i < (int)triNum ; i++ )
			vec.push_back( new InstanceTriangle( this , i , &m_Transform ) );
	}
}