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

// include the header
#include "sort.h"
#include "utility/enum.h"
#include <memory>

// pre-decleration class
class	BufferMemory;

///////////////////////////////////////////////////////////////////////
//	definition of meshloader
class	MeshLoader
{
public:
	// default constructor
	MeshLoader(){ m_MeshType = MT_NONE; }
	// destructor
	virtual ~MeshLoader(){}

	// load mesh from file
	// para 'str' : name of the file
	// para 'mem' : the memory to store
	// result     : true if loading is successful
    virtual bool LoadMesh( const std::string& str , std::shared_ptr<BufferMemory>& mem ) = 0;

	// get the type of the mesh loader
	MESH_TYPE GetMT() const { return m_MeshType; }

protected:
	// the type for the mesh
	MESH_TYPE	m_MeshType;
};