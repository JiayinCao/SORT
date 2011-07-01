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

	// generate flat normal
	if( m_pMemory->m_iNBCount == 0 )
		_genSmoothNormal();

	return true;
}

// fill buffer into vector
void TriMesh::FillTriBuf( vector<Primitive*>& vec )
{
	unsigned base = vec.size();
	if( m_bInstanced == false )
	{
		// generate the triangles
		unsigned triNum = m_pMemory->m_iTriNum;
		for( int i = 0 ; i < (int)triNum ; i++ )
			vec.push_back( new Triangle( base+i , this , i ) );
	}else
	{
		// create instance triangle
		unsigned triNum = m_pMemory->m_iTriNum;
		for( int i = 0 ; i < (int)triNum ; i++ )
			vec.push_back( new InstanceTriangle( base+i , this , i , &m_Transform ) );
	}
}

// generate normal for the triangle mesh
void TriMesh::_genFlatNormal()
{
	if( m_pMemory->m_iNBCount != 0 )
	{
		LOG_WARNING<<"Normal buffer in \""<<m_pMemory->m_filename<<"\" already exists, can't generate"<<ENDL;
		return;
	}

	// generate normal for each face
	unsigned triNum = m_pMemory->m_iTriNum;
	for( unsigned i = 0 ; i < triNum ; i++ )
	{
		unsigned offset = 3 * i;
		unsigned id0 = m_pMemory->m_IndexBuffer[offset].posIndex;
		unsigned id1 = m_pMemory->m_IndexBuffer[offset+1].posIndex;
		unsigned id2 = m_pMemory->m_IndexBuffer[offset+2].posIndex;

		// get the vertexes
		Vector v0 = m_pMemory->m_PositionBuffer[id0] - m_pMemory->m_PositionBuffer[id1];
		Vector v1 = m_pMemory->m_PositionBuffer[id2] - m_pMemory->m_PositionBuffer[id1];

		// set the normal
		Vector n = Cross( v1 , v0 );
		n.m_bNormal = true;
		n.Normalize();
		m_pMemory->m_NormalBuffer.push_back( n );

		m_pMemory->m_IndexBuffer[offset].norIndex = i;
		m_pMemory->m_IndexBuffer[offset+1].norIndex = i;
		m_pMemory->m_IndexBuffer[offset+2].norIndex = i;
	}
	m_pMemory->m_iNBCount = triNum;
}

void TriMesh::_genSmoothNormal()
{
	if( m_pMemory->m_iNBCount != 0 )
	{
		LOG_WARNING<<"Normal buffer in \""<<m_pMemory->m_filename<<"\" already exists, can't generate"<<ENDL;
		return;
	}

	// generate flat normal first
	_genFlatNormal();

	// get the adjencency information
	vector<unsigned>* adjacency = new vector<unsigned>[m_pMemory->m_iVBCount];
	unsigned triNum = m_pMemory->m_iTriNum;
	for( unsigned i = 0 ; i < triNum ; i++ )
	{
		unsigned offset = 3 * i;
		unsigned id0 = m_pMemory->m_IndexBuffer[offset].posIndex;
		unsigned id1 = m_pMemory->m_IndexBuffer[offset+1].posIndex;
		unsigned id2 = m_pMemory->m_IndexBuffer[offset+2].posIndex;

		adjacency[id0].push_back( i );
		adjacency[id1].push_back( i );
		adjacency[id2].push_back( i );

		m_pMemory->m_IndexBuffer[offset].norIndex = m_pMemory->m_IndexBuffer[offset].posIndex;
		m_pMemory->m_IndexBuffer[offset+1].norIndex = m_pMemory->m_IndexBuffer[offset+1].posIndex;
		m_pMemory->m_IndexBuffer[offset+2].norIndex = m_pMemory->m_IndexBuffer[offset+2].posIndex;
	}

	// generate smooth normal
	vector<Vector> smoothNormal;
	for( unsigned i = 0 ; i < m_pMemory->m_iVBCount ; i++ )
	{
		Vector n;

		vector<unsigned>::iterator it = adjacency[i].begin();
		while( it != adjacency[i].end() )
		{
			n += m_pMemory->m_NormalBuffer[*it];
			it++;
		}

		if( 0 != adjacency[i].size() )
		{
			n.Normalize();
			n.m_bNormal = true;
		}

		smoothNormal.push_back(n);
	}
	m_pMemory->m_NormalBuffer = smoothNormal;

	delete[] adjacency;
}
