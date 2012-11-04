/*
   FileName:      texio.h

   Created Time:  2011-08-04 12:47:54

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_TEXIO
#define SORT_TEXIO

// include the header file
#include "sort.h"
#include "utility/enum.h"

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
	TexIO(){m_TexType=TT_END;}
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
	virtual bool Read( const string& str , ImgMemory* tex )
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

#endif
