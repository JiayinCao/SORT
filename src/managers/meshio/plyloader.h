/*
 * filename :	plyloader.h
 *
 * programmer :	Cao Jiayin
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

// private method
	// close ply file
	void _closePly( PlyFile* plyfile );

	// release element
	void _releaseElement( PlyElement* element );
};

#endif