/*
 * filename :	meshmanager.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "meshmanager.h"
#include "utility/strhelper.h"
#include "meshio/objloader.h"
#include "meshio/plyloader.h"
#include "geometry/trimesh.h"
#include "geometry/triangle.h"
#include "utility/path.h"
#include "bsdf/bsdf.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(MeshManager);

// initialize the manager
void MeshManager::_init()
{
	// register the mesh loaders
	m_MeshLoader.push_back( new ObjLoader() );
	m_MeshLoader.push_back( new PlyLoader() );
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
			LOG_ERROR<<"There "<<isare<<" still "<<buf_it->second->reference<<refer<<" pointing to \""<<buf_it->first<<"\"."<<CRASH;
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
bool MeshManager::LoadMesh( const string& filename , TriMesh* mesh )
{
	// get full resource filename
	string str = GetFullPath( filename );

	// get the mesh type
	MESH_TYPE type = MeshTypeFromStr( str );

	// find the mesh memory first
	map< string , BufferMemory* >::const_iterator it = m_Buffers.find( str );
	while( it != m_Buffers.end() )
	{
		// create another instance of the mesh
		mesh->m_bInstanced = true;

		// copy the memory
		mesh->m_pMemory = it->second;

		// update the transform
		mesh->m_Transform = mesh->m_Transform * Inverse(it->second->m_pPrototype->m_Transform);
		
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

		// reset count
		mem->CalculateCount();

		// set the pointer
		if( read )
		{
			// apply the transformation
			mem->ApplyTransform( mesh );

			// if there is no normal or texture coordinate or tagent , generate them
			// because the rendering method requires all of the data
			mem->GenSmoothNormal();
			mem->GenTexCoord();
			mem->GenSmoothTagent();

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

// generate normal for the triangle mesh
void BufferMemory::_genFlatNormal()
{
	if( m_iNBCount != 0 )
		return;

	// generate the triangles
	unsigned totalTriNum = 0;
	unsigned trunkNum = m_TrunkBuffer.size();
	unsigned base = 0;
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_TrunkBuffer[i];
		for( unsigned k = 0 ; k < trunk->m_iTriNum ; k++ )
		{
			unsigned offset = 3*k;
			unsigned id0 = trunk->m_IndexBuffer[offset].posIndex;
			unsigned id1 = trunk->m_IndexBuffer[offset+1].posIndex;
			unsigned id2 = trunk->m_IndexBuffer[offset+2].posIndex;

			// get the vertexes
			Vector v0 = m_PositionBuffer[id0] - m_PositionBuffer[id1];
			Vector v1 = m_PositionBuffer[id2] - m_PositionBuffer[id1];

			// set the normal
			Vector n = Cross( v1 , v0 );
			n.m_bNormal = true;
			n.Normalize();
			m_NormalBuffer.push_back( n );

			trunk->m_IndexBuffer[offset].norIndex = base+k;
			trunk->m_IndexBuffer[offset+1].norIndex = base+k;
			trunk->m_IndexBuffer[offset+2].norIndex = base+k;
		}
		totalTriNum += trunk->m_iTriNum;
		base += trunk->m_iTriNum;
	}
	m_iNBCount = totalTriNum;
}

void BufferMemory::GenSmoothNormal()
{
	if( m_iNBCount != 0 )
		return;

	// generate flat normal first
	_genFlatNormal();

	// get the adjacency information
	vector<unsigned>* adjacency = new vector<unsigned>[m_iVBCount];
	// generate the triangles
	unsigned trunkNum = m_TrunkBuffer.size();
	unsigned base = 0;
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_TrunkBuffer[i];
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
	for( unsigned i = 0 ; i < m_iVBCount ; i++ )
	{
		Vector n;

		vector<unsigned>::iterator it = adjacency[i].begin();
		while( it != adjacency[i].end() )
		{
			n += m_NormalBuffer[*it];
			it++;
		}

		if( 0 != adjacency[i].size() )
		{
			n.Normalize();
			n.m_bNormal = true;
		}

		smoothNormal.push_back(n);
	}
	m_NormalBuffer = smoothNormal;
	m_iNBCount = m_NormalBuffer.size();

	delete[] adjacency;
}

// generate tagent for the triangle mesh
void BufferMemory::GenSmoothTagent()
{
	if( m_iTeBcount != 0 )
		return;

	// generate tagent for each triangle
	vector<Vector> tagents;
	const unsigned trunkNum = m_TrunkBuffer.size();
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_TrunkBuffer[i];
		for( unsigned k = 0 ; k < trunk->m_iTriNum ; k++ )
			tagents.push_back( _genTagentForTri( trunk , k ) );
	}

	// set the adjacency information
	vector<unsigned>* adjacency = new vector<unsigned>[m_iNBCount];
	unsigned base = 0;
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_TrunkBuffer[i];
		for( unsigned k = 0 ; k < trunk->m_iTriNum ; k++ )
		{
			unsigned offset = 3 * k;
			unsigned id0 = trunk->m_IndexBuffer[offset].norIndex;
			unsigned id1 = trunk->m_IndexBuffer[offset+1].norIndex;
			unsigned id2 = trunk->m_IndexBuffer[offset+2].norIndex;

			adjacency[id0].push_back( base + k );
			adjacency[id1].push_back( base + k );
			adjacency[id2].push_back( base + k );
		}
		base += trunk->m_iTriNum;
	}

	// generate smooth normal
	for( unsigned i = 0 ; i < m_iNBCount ; i++ )
	{
		Vector t;

		vector<unsigned>::iterator it = adjacency[i].begin();
		while( it != adjacency[i].end() )
		{
			t += tagents[*it];
			it++;
		}

		if( 0 != adjacency[i].size() )
			t.Normalize();

		m_TangentBuffer.push_back(t);
	}
	m_iTeBcount = m_TangentBuffer.size();

	// free the memory
	delete[] adjacency;
}

// generate tagent vector for a triangle
Vector BufferMemory::_genTagentForTri( const Trunk* trunk , unsigned k ) const
{
	unsigned offset = 3 * k;
	unsigned pid0 = trunk->m_IndexBuffer[offset].posIndex;
	unsigned pid1 = trunk->m_IndexBuffer[offset+1].posIndex;
	unsigned pid2 = trunk->m_IndexBuffer[offset+2].posIndex;

	// get three vertexes
	const Point& p0 = m_PositionBuffer[pid0] ;
	const Point& p1 = m_PositionBuffer[pid1] ;
	const Point& p2 = m_PositionBuffer[pid2] ;

	unsigned tid0 = 2 * trunk->m_IndexBuffer[offset].texIndex;
	unsigned tid1 = 2 * trunk->m_IndexBuffer[offset+1].texIndex;
	unsigned tid2 = 2 * trunk->m_IndexBuffer[offset+2].texIndex;

	const float u0 = m_TexCoordBuffer[tid0];
	const float u1 = m_TexCoordBuffer[tid1];
	const float u2 = m_TexCoordBuffer[tid2];
	const float v0 = m_TexCoordBuffer[tid0+1];
	const float v1 = m_TexCoordBuffer[tid1+1];
	const float v2 = m_TexCoordBuffer[tid2+1];

	float du1 = u0 - u2;
	float du2 = u1 - u2;
	float dv1 = v0 - v2;
	float dv2 = v1 - v2;
	Vector dp1 = p0 - p2;
	Vector dp2 = p1 - p2;

	float determinant = du1 * dv2 - dv1 * du2 ;
	if( determinant == 0.0f )
	{
		LOG_WARNING<<"There is a triangle containing three vertexes with same texture coodinate , can't generate shading coordinate correctly."<<ENDL;
		Vector n = Normalize( p0 - p1 );
		Vector t0 , t1;
		CoordinateSystem( n , t0 , t1 );
		return t0;
	}

	float invdet = 1.0f / determinant;
	
	return ( dv2 * dp1 - dv1 * dp2 ) * invdet;
}

// generate texture coordinate
void BufferMemory::GenTexCoord()
{
	if( m_iTBCount != 0 )
		return;

	// find the center of the mesh
	Point center;
	vector<Point>::iterator it = m_PositionBuffer.begin();
	while( it != m_PositionBuffer.end() )
	{
		center = center + *it;
		it++;
	}
	center /= (float)m_PositionBuffer.size();

	it = m_PositionBuffer.begin();
	while( it != m_PositionBuffer.end() )
	{
		Vector diff = *it - center;
		diff.Normalize();

		float u = SphericalTheta( diff ) / PI ;
		float v = SphericalPhi( diff ) / PI * 0.5f;

		m_TexCoordBuffer.push_back( u );
		m_TexCoordBuffer.push_back( v );

		it++;
	}
	m_iTBCount = m_TexCoordBuffer.size();

	// set the texture coordinate index
	unsigned trunkNum = m_TrunkBuffer.size();
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		Trunk* trunk = m_TrunkBuffer[i];
		for( unsigned k = 0 ; k < trunk->m_iTriNum ; k++ )
		{
			unsigned offset = 3*k;
			trunk->m_IndexBuffer[offset].texIndex = trunk->m_IndexBuffer[offset].posIndex;
			trunk->m_IndexBuffer[offset+1].texIndex = trunk->m_IndexBuffer[offset+1].posIndex;
			trunk->m_IndexBuffer[offset+2].texIndex = trunk->m_IndexBuffer[offset+2].posIndex;
		}
	}
}