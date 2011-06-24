/*
 * filename :	meshmanager.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "meshmanager.h"
#include "meshio/objloader.h"
#include "geometry/trimesh.h"
#include "geometry/triangle.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(MeshManager);

// initialize the manager
void MeshManager::_init()
{
	// register the mesh loaders
	m_MeshLoader.push_back( new ObjLoader() );
}

// release the memory
void MeshManager::_release()
{
	// unregister the mesh loader
	vector<MeshLoader*>::iterator it = m_MeshLoader.begin();
	while( it != m_MeshLoader.end() )
	{
		delete *it;
		it++;
	}
	m_MeshLoader.clear();

	map< string , BufferMemory* >::iterator buf_it = m_Buffers.begin();
	while( buf_it != m_Buffers.end() )
	{
		if( buf_it->second->reference > 0 )
		{
			string isare = (buf_it->second->reference>1)?"is":"are";
			string refer = (buf_it->second->reference>1)?" reference":" references";
			LOG_ERROR<<"There "<<isare<<" still "<<buf_it->second->reference<<refer<<" pointing to "<<buf_it->first<<"."<<CRASH;
		}

		// delete the memory
		delete buf_it->second;

		buf_it++;
	}
	m_Buffers.clear();
}

// get the mesh loader
MeshLoader*	MeshManager::_getMeshLoader( MESH_TYPE type ) const
{
	// unregister the mesh loader
	vector<MeshLoader*>::const_iterator it = m_MeshLoader.begin();
	while( it != m_MeshLoader.end() )
	{
		if( (*it)->GetMT() == type )
			return *it;
		it++;
	}

	return 0;
}

// load the mesh from file
bool MeshManager::LoadMesh( const string& str , TriMesh* mesh , MESH_TYPE type )
{
	// find the mesh memory first
	map< string , BufferMemory* >::const_iterator it = m_Buffers.find( str );
	while( it != m_Buffers.end() )
	{
		// create another instance of the mesh
		mesh->m_bInstanced = true;

		// copy the memory
		mesh->m_pMemory = it->second;

		// update the transform
		mesh->m_Transform = Inverse(it->second->m_pPrototype->m_Transform) * mesh->m_Transform;
		
		return true;
	}
	
	// get the mesh loader first
	MeshLoader* loader = _getMeshLoader( type );

	bool read = false;
	if( loader )
	{
		// create the new memory
		BufferMemory* mem = new BufferMemory();

		// load the mesh from file
		read = loader->LoadMesh( str , mem );

		// set the pointer
		if( read )
		{
			// apply the transformation
			mem->ApplyTransform( mesh );

			mesh->m_bInstanced = false;
			mesh->m_pMemory = mem;

			// and insert it into the map
			m_Buffers.insert( make_pair( str , mem ) );
		}else
		{
			LOG_WARNING<<"Can't load file \""<<str<<"\"."<<ENDL;
			delete mem;
		}
	}

	return read;
}

// apply transform
void BufferMemory::ApplyTransform( TriMesh* mesh )
{
	vector<Point>::iterator p_it = m_PositionBuffer.begin();
	while( p_it != m_PositionBuffer.end() )
	{
		*p_it = (mesh->m_Transform)(*p_it);
		p_it++;
	}
	vector<Vector>::iterator n_it = m_NormalBuffer.begin();
	while( n_it != m_NormalBuffer.end() )
	{
		*n_it = (mesh->m_Transform)(*n_it);
		n_it++;
	}
	m_pPrototype = mesh;
}