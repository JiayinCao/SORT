/*
 * filename :	trimesh.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "trimesh.h"
#include "../sort.h"
#include "normal.h"
#include "point.h"

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
	m_pVertexBuffer = 0;
	m_pNormalBuffer = 0;
	m_pIndexBuffer = 0;
	m_iVBCount = 0;
	m_iNBCount = 0;
	m_iIBCount = 0;
	m_iTriNum = 0;
}

// release the memory
void TriMesh::Release()
{
	SAFE_DELETE_ARRAY( m_pVertexBuffer );
	SAFE_DELETE_ARRAY( m_pNormalBuffer );
	SAFE_DELETE_ARRAY( m_pIndexBuffer );
}
