/*
   FileName:      meshloader.h

   Created Time:  2011-08-04 12:48:02

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MESHLOADER
#define	SORT_MESHLOADER

// include the header
#include "sort.h"
#include "utility/enum.h"

// pre-decleration class
class	BufferMemory;

///////////////////////////////////////////////////////////////////////
//	definition of meshloader
class	MeshLoader
{
// public method
public:
	// default constructor
	MeshLoader(){ m_MeshType = MT_END; }
	// destructor
	~MeshLoader(){}

	// load mesh from file
	// para 'str' : name of the file
	// para 'mem' : the memory to store
	// result     : true if loading is successful
	virtual bool LoadMesh( const string& str , BufferMemory* mem ) = 0;

	// get the type of the mesh loader
	MESH_TYPE GetMT() const { return m_MeshType; }

// protected data
protected:
	// the type for the mesh
	MESH_TYPE	m_MeshType;
};

#endif
