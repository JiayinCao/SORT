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

	m_iVBCount = 3;
	m_iIBCount = 3;
	m_iTriNum = 1;

	//temp , check the triangle intersectionn algorithm
	m_pVertexBuffer = new Point[m_iVBCount];
	m_pIndexBuffer = new unsigned[m_iIBCount];

	m_pVertexBuffer[0] = Point( 0 , 0 , 0 );
	m_pVertexBuffer[1] = Point( 1 , 0 , 0 );
	m_pVertexBuffer[2] = Point( 1 , 0 , 1 );

	m_pIndexBuffer[0] = 0;
	m_pIndexBuffer[1] = 1;
	m_pIndexBuffer[2] = 2;

	m_TriList.push_back( Triangle( this , m_pVertexBuffer , m_pIndexBuffer ) );
}

// release the memory
void TriMesh::Release()
{
	SAFE_DELETE_ARRAY( m_pVertexBuffer );
	SAFE_DELETE_ARRAY( m_pNormalBuffer );
	SAFE_DELETE_ARRAY( m_pIndexBuffer );
}