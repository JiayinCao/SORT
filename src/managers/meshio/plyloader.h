/*
   FileName:      plyloader.h

   Created Time:  2011-08-04 12:48:14

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_PLYLOADER
#define	SORT_PLYLOADER

// include the header file
#include "meshloader.h"

// pre-decleration
struct PlyFile;
struct PlyElement;

struct PlyIndex
{
	unsigned	count;
	unsigned*	index;

	PlyIndex()
	{
		count = 0;
		index = 0;
	}
	~PlyIndex()
	{
		count = 0;
		delete[] index;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////
// defination of plyloader
class	PlyLoader : public MeshLoader
{
// public method
public:
	// default constructor
	PlyLoader(){m_MeshType = MT_PLY;}
	// destructor
	~PlyLoader(){}

	// load mesh from file
	// para 'str' : name of the file
	// para 'mem' : the memory to store
	// result     : true if loading is successful
	bool	LoadMesh( const string& str , BufferMemory* mem );
};

#endif