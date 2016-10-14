/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_TRIMESH
#define	SORT_TRIMESH

// include the headers
#include <vector>
#include "primitive.h"
#include "utility/referencecount.h"
#include "managers/meshmanager.h"
#include "math/transform.h"

class	Material;

//////////////////////////////////////////////////////////////////////////////////
//	definition of trimesh
class TriMesh
{
// public method
public:
	// default constructor
	TriMesh(const string& name);
	// destructor
	~TriMesh();

	// load the mesh from file
	// para 'str'  : the name of the input file
	// para 'transform' : the transformation of the mesh
	// para 'type' : the type of the mesh file , default value is obj
	// result      : 'true' if loading is successful
	bool LoadMesh( const string& str , Transform& transform );

	// fill buffer into vector
	// para 'vec' : the buffer to filled
	void FillTriBuf( vector<Primitive*>& vec );

	// reset material
	// para 'setname' : the subset to set material
	// para 'matname' : the material name
	void ResetMaterial( const string& setname , const string& matname );

// private field
public:
	// the name of the model
	const string	m_Name;

	// the memory for the mesh
	Reference<BufferMemory> m_pMemory;

	// the transformation of the mesh
	Transform		m_Transform;

	// whether the mesh is instanced
	bool			m_bInstanced;

	// the materials for instanced mesh
	Reference<Material>*	m_pMaterials;

// private method
	// initialize default data
	void	_init();
	// release the default data
	void	_release();
	// get the subset of the mesh
	int		_getSubsetID( const string& setname );
	// copy materials
	void	_copyMaterial();

// set friend class
friend	class	MeshManager;
friend	class	Triangle;
};

#endif
