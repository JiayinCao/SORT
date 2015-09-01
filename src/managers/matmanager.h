/*
   FileName:      matmanager.h

   Created Time:  2011-08-04 12:47:25

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_MATMANAGER
#define	SORT_MATMANAGER

// include header file
#include "utility/singleton.h"
#include <string>
#include <map>

// pre-decleration
class Material;
class TiXmlElement;
class MaterialNode;

/////////////////////////////////////////////////////////////////////////////
//	definition of material manager
//	desc :	Material manager is a singleton. All of the materials in the system
//			are parsed from file and located in a single pool. And it's 
//			responsible for deallocate all of the material memory.
//	note :	There will be textuers in most of materials. when materials are
//			deleted the bind texture is also deleted. In another words ,
//			material system is also responsible for deallocating textures.
class	MatManager : public Singleton<MatManager>
{
// public method
public:
	// default constructor
	MatManager();
	// destructor
	~MatManager();

	// find specific material
	// para 'mat_name' : the name for the material
	// result          : the material with specific material , default material
	//					 if there is no material with the name.
	Material*	FindMaterial( const string& mat_name ) const;

	// get default material
	Material*	GetDefaultMat() const;

	// parse material file and add the materials into the manager
	// para 'str' : name of the material file
	// result     : the number of materials in the file
	unsigned	ParseMatFile( const string& str );

	// get material number
	unsigned	GetMatCount() const;

// private field
private:
	// material pool
	map< string , Material* >	m_matPool;

	// the default material
	Material*	m_Default;

	// clear the material pool
	void _clearMatPool();

	// parse material nodes
	MaterialNode*	_parseMaterialNode( TiXmlElement* prop , const string& name );

	friend class Singleton<MatManager>;
};

#endif