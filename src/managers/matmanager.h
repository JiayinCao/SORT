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

#ifndef SORT_MATMANAGER
#define	SORT_MATMANAGER

// include header file
#include "utility/singleton.h"
#include <string>
#include <map>
#include "material/matte.h"

// pre-decleration
class TiXmlElement;

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
	Material*	GetDefaultMat();

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
	Matte	m_Default;

	// clear the material pool
	void _clearMatPool();

	friend class Singleton<MatManager>;
};

#endif
