/*
 * filename :	trimesh.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TRIMESH
#define	SORT_TRIMESH

// include the headers
#include <vector>
#include "primitive.h"
#include "utility/referencecount.h"
#include "managers/meshmanager.h"

// pre-declera classes
class	Vector;
class	Point;

//////////////////////////////////////////////////////////////////////////////////
//	definition of trimesh
class TriMesh
{
// public method
public:
	// default constructor
	TriMesh();
	// destructor
	~TriMesh();

	// release the memory
	void Release();

	// load the mesh from file
	// para 'str'  : the name of the input file
	// para 'type' : the type of the mesh file , default value is obj
	// result      : 'true' if loading is successful
	bool LoadMesh( const string& str , MESH_TYPE type = MT_OBJ );

// private field
public:
	// the memory for the mesh
	Reference<BufferMemory> m_pMemory;

	// the triangle list
	vector<Primitive*>		m_triBuffer;

// private method
	// initialize default data
	void	_init();

// set friend class
friend	class	MeshManager;
friend	class	Triangle;
};

#endif
