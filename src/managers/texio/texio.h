/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

// include the header file
#include "sort.h"
#include "utility/enum.h"
#include <memory>

// pre-declare texture class
class Texture;
class ImgMemory;

////////////////////////////////////////////////////////////////////////////
// definition of TexIO
// TexIO is an abstract class that is responsible for outputing texture
// in different ways , such as saving the texture as an image file or 
// popping an window showing the texture.
class	TexIO
{
// public method
public:
	// default constructor
	TexIO(){m_TexType=TT_NONE;}
	// constructor from a type
	// para 'tot' : texture output type
	// note	      :	actually set tot_type in TexIO gains nothing
	TexIO( TEX_TYPE tt ) { m_TexType = tt; }
	// destructor
	virtual ~TexIO() {}

	// output the file in different ways
	// para 'str' : the name of the output entity
	// para 'tex' : the texture for outputing
	// result     : 'true' if outputing is successed
	virtual bool Write( const string& str , const Texture* tex ) = 0;

	// read data from file
	// para 'str' : the name of the input entity
	// para 'tex' : the texture for aside the memory
	// result     :	return true if loading is successful
	// note       : it's not a pure function because some shower doesn't need a reading func
    virtual bool Read( const string& str , std::shared_ptr<ImgMemory>& tex )
	{
		return false;
	}

	// get tot type
	TEX_TYPE GetTT()
	{
		return m_TexType;
	}

// protected data
protected:
	// the type of the outputer
	TEX_TYPE m_TexType;
};
