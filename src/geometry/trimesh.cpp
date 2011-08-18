/*
   FileName:      trimesh.cpp

   Created Time:  2011-08-04 12:51:13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
TriMesh::TriMesh( const string& name ):m_Name(name)
{
	// initialize default data
	_init();
}
// destructor
TriMesh::~TriMesh()
{
	_release();
}

// initialize default data
void TriMesh::_init()
{
	m_pMemory = 0;
	m_pMaterials = 0;
	m_bEmissive = false;
}

// release the default data
void TriMesh::_release()
{
	SAFE_DELETE_ARRAY(m_pMaterials);
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

	_copyMaterial();
	return true;
}

// copy materials
void TriMesh::_copyMaterial()
{
	SAFE_DELETE_ARRAY(m_pMaterials);

	unsigned trunk_size = m_pMemory->m_TrunkBuffer.size();
	m_pMaterials = new Reference<Material>[trunk_size];
	
	for( unsigned i = 0 ; i < trunk_size ; ++i )
	{
		if( m_pMemory->m_TrunkBuffer[i]->m_mat )
			m_pMaterials[i] = m_pMemory->m_TrunkBuffer[i]->m_mat;
		else
			m_pMaterials[i] = MatManager::GetSingleton().GetDefaultMat();
		m_pMemory->m_TrunkBuffer[i]->m_mat = 0;
	}
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
				vec.push_back( new Triangle( base+k , this , &(m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer[3*k]) , m_pMaterials[i] , m_bEmissive) );
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
				vec.push_back( new InstanceTriangle( base+k , this , &(m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer[3*k]) , &m_Transform , m_pMaterials[i] , m_bEmissive) );
			base += m_pMemory->m_TrunkBuffer[i]->m_IndexBuffer.size() / 3;
		}
	}
}

// reset material
void TriMesh::ResetMaterial( const string& setname , const string& matname )
{
	// get the material first
	Material* mat = MatManager::GetSingleton().FindMaterial( matname );
	if( mat == 0 )
		LOG_WARNING<<"There is no such a material named \'"<<matname<<"\'."<<ENDL;

	// if there is no set name , all of the sets are set the material with the name of 'matname'
	if( setname.empty() )
	{
		unsigned size = m_pMemory->m_TrunkBuffer.size();
		for( unsigned i = 0 ; i < size ; ++i )
			m_pMaterials[i] = mat;
		return;
	}

	int id = _getSubsetID( setname );
	if( id < 0 )
	{
		LOG_WARNING<<"There is no such subset named "<<setname<<ENDL;
		return;
	}
	m_pMaterials[id] = mat;
}

// get the subset of the mesh
int TriMesh::_getSubsetID( const string& setname )
{
	int size = (int)m_pMemory->m_TrunkBuffer.size();
	for( int i = 0 ; i < size ; ++i )
	{
		if( m_pMemory->m_TrunkBuffer[i]->name == setname )
			return i;
	}
	return -1;
}

// set emissive
void TriMesh::SetEmission( Light* l )
{
	unsigned size = m_pMemory->m_TrunkBuffer.size();
	for( unsigned i = 0 ; i < size ; ++i )
		m_pMaterials[i]->BindLight( l );
	m_bEmissive = true;
}