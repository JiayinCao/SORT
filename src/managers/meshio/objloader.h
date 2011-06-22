/*
 * filename :	objloader.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_OBJLOADER
#define	SORT_OBJLOADER

// include the header file
#include "meshloader.h"

//////////////////////////////////////////////////////////////////////
//	definition of objloader
class	ObjLoader : public MeshLoader
{
// public method
public:
	// default constructor
	ObjLoader(){m_MeshType = MT_OBJ;}
	// destructor
	~ObjLoader(){}

	// load mesh from file
	// para 'str' : name of the file
	// para 'mem' : the memory to store
	// result     : true if loading is successful
	bool	LoadMesh( const string& str , BufferMemory* mem ); 
};

#endif
