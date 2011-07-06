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
		unsigned trunkNum = m_pMemory->m_TrunkBuffer.size();
		for( unsigned i = 0 ; i < trunkNum ; i++ )
		{
			unsigned trunkTriNum = m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
			for( unsigned k = 0 ; k < trunkTriNum ; k++ )
				vec.push_back( new Triangle( base+k , this , &(m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer[3*k]) ) );
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
				vec.push_back( new InstanceTriangle( base+k , this , &(m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer[3*k]) , &m_Transform ) );
			base += m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
		}
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

	// generate the triangles
	unsigned totalTriNum = 0;
	unsigned trunkNum = m_pMemory->m_TrunkBuffer.size();
	unsigned base = 0;
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_pMemory->m_TrunkBuffer[i];
		for( unsigned k = 0 ; k < trunk->m_iTriNum ; k++ )
		{
			unsigned offset = 3*k;
			unsigned id0 = trunk->m_IndexBuffer[offset].posIndex;
			unsigned id1 = trunk->m_IndexBuffer[offset+1].posIndex;
			unsigned id2 = trunk->m_IndexBuffer[offset+2].posIndex;

			// get the vertexes
			Vector v0 = m_pMemory->m_PositionBuffer[id0] - m_pMemory->m_PositionBuffer[id1];
			Vector v1 = m_pMemory->m_PositionBuffer[id2] - m_pMemory->m_PositionBuffer[id1];

			// set the normal
			Vector n = Cross( v1 , v0 );
			n.m_bNormal = true;
			n.Normalize();
			m_pMemory->m_NormalBuffer.push_back( n );

			trunk->m_IndexBuffer[offset].norIndex = base+k;
			trunk->m_IndexBuffer[offset+1].norIndex = base+k;
			trunk->m_IndexBuffer[offset+2].norIndex = base+k;
		}
		totalTriNum += trunk->m_iTriNum;
		base += trunk->m_iTriNum;
	}
	m_pMemory->m_iNBCount = totalTriNum;
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
	// generate the triangles
	unsigned trunkNum = m_pMemory->m_TrunkBuffer.size();
	unsigned base = 0;
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_pMemory->m_TrunkBuffer[i];
		for( unsigned k = 0 ; k < trunk->m_iTriNum ; k++ )
		{
			unsigned offset = 3 * k;
			unsigned id0 = trunk->m_IndexBuffer[offset].posIndex;
			unsigned id1 = trunk->m_IndexBuffer[offset+1].posIndex;
			unsigned id2 = trunk->m_IndexBuffer[offset+2].posIndex;

			adjacency[id0].push_back( base + k );
			adjacency[id1].push_back( base + k );
			adjacency[id2].push_back( base + k );

			trunk->m_IndexBuffer[offset].norIndex = trunk->m_IndexBuffer[offset].posIndex;
			trunk->m_IndexBuffer[offset+1].norIndex = trunk->m_IndexBuffer[offset+1].posIndex;
			trunk->m_IndexBuffer[offset+2].norIndex = trunk->m_IndexBuffer[offset+2].posIndex;
		}
		base += trunk->m_iTriNum;
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
	m_pMemory->m_iNBCount = m_pMemory->m_NormalBuffer.size() / 3;

	delete[] adjacency;
}
