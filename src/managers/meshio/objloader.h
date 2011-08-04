/*
   FileName:      objloader.h

   Created Time:  2011-08-04 12:48:09

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
