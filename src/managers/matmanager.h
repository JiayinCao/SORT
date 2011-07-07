/*
 * filename :	matmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_MATMANAGER
#define	SORT_MATMANAGER

// include header file
#include "utility/singleton.h"
#include <string>
#include <map>

// pre-decleration
class Material;

/////////////////////////////////////////////////////////////////////////////
//	definition of material manager
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
	// result          : the material with specific material , 0 if there is 
	//					 no material with the name.
	Material*	FindMaterial( const string& mat_name ) const;

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

	// the registered type
	map< string , Material* >	m_matType;

	// register all of the materials
	void _registerMaterials();
	// clear registered types
	void _unregisterMaterials();
	// clear the material pool
	void _clearMatPool();
	// create material
	// para 'str' : material type
	// result     : the material with specific type
	Material* _createMaterial( const string& str );

	friend class Singleton<MatManager>;
};

#endif