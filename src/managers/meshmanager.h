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

#pragma once

// include the headers
#include "sort.h"
#include "utility/singleton.h"
#include "utility/enum.h"
#include "utility/define.h"
#include <vector>
#include <unordered_map>
#include "math/point.h"
#include "math/vector3.h"
#include "math/transform.h"
#include "material/material.h"
#include <memory>
#include "stream/stream.h"

#ifdef SORT_IN_WINDOWS
// Suppress the warning of duplicated IStream definition.
#pragma warning(disable:4099)
#endif

class MeshLoader;
class MeshEntity;

// index for a vertex
struct VertexIndex
{
	int	posIndex;
	int	norIndex;
	int	texIndex;

	VertexIndex()
	{
		posIndex = -1;
		norIndex = -1;
		texIndex = -1;
	}
	~VertexIndex(){}
};

// trunk (submesh)
// a trunk only contains index information
class Trunk
{
public:
	// the name for the current trunk
	std::string	name;
	// index buffer
	std::vector<VertexIndex>	m_IndexBuffer;
	// the triangle number
	unsigned	m_iTriNum;
	// the material
    std::shared_ptr<Material>	m_mat;

	// constructor
	// para 'str' : name for the trunk
	Trunk( const std::string& str = "" ) : name(str)
	{ m_iTriNum = 0; m_mat = 0; }
};

// the buffer memory for the mesh
class BufferMemory : public SerializableObject
{
public:
	// the vertex buffer
	std::vector<Point>	m_PositionBuffer;
	// the normal buffer
	std::vector<Vector>	m_NormalBuffer;
	// the tagent buffer
	std::vector<Vector>	m_TangentBuffer;
	// the texture coordinate buffer
	std::vector<float>	m_TexCoordBuffer;
	// the trunk buffer
    std::vector<Trunk>	m_TrunkBuffer;
	// the size for three buffers
	unsigned		m_iVBCount , m_iNBCount , m_iTeBcount , m_iTBCount;
	// the number of triangles 
	unsigned		m_iTriNum;
	// the trunk number
	unsigned		m_iTrunkNum;
	// the tri mesh entity
	MeshEntity*		m_pPrototypeEntity;
	// the name for the file
	std::string		m_filename;
	// whether the source file has normal or texture coordinate
	bool			m_hasInitTexCoord;
	bool			m_hasInitNormal;

	// set default data for the buffer memory
	BufferMemory()
	{
		m_iVBCount = 0;
		m_iNBCount = 0;
		m_iTeBcount = 0;
		m_iTBCount = 0;
		m_iTriNum = 0;
		m_pPrototypeEntity = nullptr;
		m_iTrunkNum = 0;
		m_hasInitTexCoord = false;
		m_hasInitNormal = false;
	}

	// apply transform
	void ApplyTransform( MeshEntity* mesh );

	// calculate buffer number
	void CalculateCount()
	{
		m_iVBCount = (unsigned)m_PositionBuffer.size();
		m_iTBCount = (unsigned)m_TexCoordBuffer.size();
		m_iNBCount = (unsigned)m_NormalBuffer.size();
		m_iTeBcount = (unsigned)m_TangentBuffer.size();
		m_iTrunkNum = (unsigned)m_TrunkBuffer.size();
		m_iTriNum = 0;
		auto it = m_TrunkBuffer.begin();
		while( it != m_TrunkBuffer.end() )
		{
			it->m_iTriNum = ((unsigned)it->m_IndexBuffer.size()) / 3;
			m_iTriNum += it->m_iTriNum;
			it++;
		}
	}

	// generate normal for the triangle mesh
	void	GenSmoothNormal();
	// generate tagent for the triangle mesh
	void	GenSmoothTagent();
	// generate texture coordinate
	void	GenTexCoord();

	// serialization interface for BufferMemory
    void    Serialize( IStreamBase& stream ) override;
	void    Serialize( OStreamBase& stream ) override;

private:
	void	_genFlatNormal();
    Vector	_genTagentForTri( const Trunk& trunk , unsigned k  ) const;
};

/////////////////////////////////////////////////////////////////////////
//	definition of mesh manager
//	desc :	A mesh manager manages all of the geometry data used in the 
//			rendering system. The meshes are not located here. Two meshes
//			will share the same geometry memory if they are loaded from
//			from the same model file, which means intancing is supported
//			in the current system.
class	MeshManager : public Singleton<MeshManager>
{
public:
	// Temporary
	bool LoadMesh( const std::string& str , MeshEntity* mesh );

private:
	// the mesh loaders
    std::vector<std::shared_ptr<MeshLoader>>	m_MeshLoader;

	// the memory for meshes
    std::unordered_map< std::string , std::shared_ptr<BufferMemory> > m_Buffers;

private:
	// default constructor
    MeshManager();
    
	// get the mesh loader
	std::shared_ptr<MeshLoader>	_getMeshLoader( MESH_TYPE type ) const;

	friend class Singleton<MeshManager>;
};
