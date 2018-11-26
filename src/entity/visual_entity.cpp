/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "visual_entity.h"
#include "managers/matmanager.h"
#include "geometry/scene.h"
#include "shape/triangle.h"

void MeshEntity::FillScene( Scene& scene ){
    unsigned base = (unsigned)scene.m_primitiveBuf.size();

	// generate the triangles
	unsigned trunkNum = (unsigned)m_memory->m_TrunkBuffer.size();
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		unsigned trunkTriNum = (unsigned)m_memory->m_TrunkBuffer[i].m_IndexBuffer.size() / 3;
		for( unsigned k = 0 ; k < trunkTriNum ; k++ ){
			Triangle* tri = new Triangle( this , &(m_memory->m_TrunkBuffer[i].m_IndexBuffer[3*k]) );
			scene.m_primitiveBuf.push_back( new Primitive( m_Materials[i] , tri ) );
		}
		base += (unsigned)(m_memory->m_TrunkBuffer[i].m_IndexBuffer.size() / 3);
	}
}

void MeshEntity::Serialize( IStreamBase& stream ){
	m_memory->Serialize(stream);
}

void MeshEntity::Serialize( OStreamBase& stream ){
	m_memory->Serialize(stream);
}

// ------------------------------------------------------------------------
// Temporary solution before serialization is done.
bool MeshEntity::LoadMesh( const std::string& filename , const Transform& transform ){
    m_transform = transform;
    bool flag = MeshManager::GetSingleton().LoadMesh( filename , this );
    if( false == flag )
        return false;

    _copyMaterial();

    return true;
}
void MeshEntity::ResetMaterial( const std::string& setname , const std::string& matname )
{
	// get the material first
    auto mat = MatManager::GetSingleton().FindMaterial( matname );
	if( mat == nullptr )
        slog( WARNING , MATERIAL , stringFormat( "Material %s doesn't exist." , matname.c_str() ) );

	// if there is no set name , all of the sets are set the material with the name of 'matname'
	if( setname.empty() )
	{
		unsigned size = (unsigned)m_memory->m_TrunkBuffer.size();
		for( unsigned i = 0 ; i < size ; ++i )
			m_Materials[i] = mat;
		return;
	}

	int id = _getSubsetID( setname );
	if( id < 0 ){
        slog( WARNING , MATERIAL , stringFormat( "Material subset %s in material %s doesn't exist." , matname.c_str() , matname.c_str() ) );
		return;
	}
	m_Materials[id] = mat;
}

// get the subset of the mesh
int MeshEntity::_getSubsetID( const std::string& setname )
{
	int size = (int)m_memory->m_TrunkBuffer.size();
	for( int i = 0 ; i < size ; ++i )
	{
		if( m_memory->m_TrunkBuffer[i].name == setname )
			return i;
	}
	return -1;
}

// copy materials
void MeshEntity::_copyMaterial()
{
	unsigned trunk_size = (unsigned)m_memory->m_TrunkBuffer.size();
    m_Materials.resize( trunk_size );
	
	for( unsigned i = 0 ; i < trunk_size ; ++i )
        m_Materials[i] = m_memory->m_TrunkBuffer[i].m_mat ? m_memory->m_TrunkBuffer[i].m_mat : MatManager::GetSingleton().GetDefaultMat();
}
// ------------------------------------------------------------------------
