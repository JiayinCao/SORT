/*
   FileName:      trimesh.h

   Created Time:  2011-08-04 12:51:16

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_TRIMESH
#define	SORT_TRIMESH

// include the headers
#include <vector>
#include "primitive.h"
#include "utility/referencecount.h"
#include "managers/meshmanager.h"
#include "geometry/transform.h"

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

	// get total surface area
	float	GetSurfaceArea() const;

// private field
public:
	// the name of the model
	const string	m_Name;

	// the memory for the mesh
	Reference<BufferMemory> m_pMemory;

	// the tranformation of the mesh
	Transform		m_Transform;

	// whether the mesh is instanced
	bool			m_bInstanced;

	// the triangles for the mesh
	vector<Primitive*> m_triBuffer;
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
	// generate tri buffer
	void	_genTriBuffer();

// set friend class
friend	class	MeshManager;
friend	class	Triangle;
};

#endif
