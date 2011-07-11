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
#include "managers/logmanager.h"

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
bool TriMesh::LoadMesh( const string& str , Transform& transform )
{
	// set the tranformation
	m_Transform = transform;

	// load the mesh
	bool flag = MeshManager::GetSingleton().LoadMesh( str , this );
	if( false == flag )
		return false;

	return true;
}

// fill buffer into vector
void TriMesh::FillTriBuf( vector<Primitive*>& vec )
{
	unsigned base = vec.size();
	if( m_bInstanced == false )
	{
		// generate the triangles
		unsigned trunkNum = m_pMemory->m_TrunkBuffer.size();
		for( unsigned i = 0 ; i < trunkNum ; i++ )
		{
			unsigned trunkTriNum = m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
			for( unsigned k = 0 ; k < trunkTriNum ; k++ )
				vec.push_back( new Triangle( base+k , this , &(m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer[3*k]) , m_pMemory->m_TrunkBuffer[i]->m_mat ) );
			base += m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
		}
	}else
	{
		// generate the triangles
		unsigned trunkNum = m_pMemory->m_TrunkBuffer.size();
		for( unsigned i = 0 ; i < trunkNum ; i++ )
		{
			unsigned trunkTriNum = m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
			for( unsigned k = 0 ; k < trunkTriNum ; k++ )
				vec.push_back( new InstanceTriangle( base+k , this , &(m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer[3*k]) , &m_Transform , m_pMemory->m_TrunkBuffer[i]->m_mat ) );
			base += m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
		}
	}
}