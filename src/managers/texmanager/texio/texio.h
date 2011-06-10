/*
 * filename :	texio.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_TEXIO
#define SORT_TEXIO

// texture output type
enum TEX_TYPE
{
	TT_BMP = 0,
	TT_WINDOW ,
	TT_END ,
};

// include the header file
#include "../../../sort.h"
#include "../../../texture/texture.h"

// pre-declare texture class
class Texture;
class ImageTexture;

////////////////////////////////////////////////////////////////////////////
// defination of TexIO
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
	~TexIO() {}

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
	virtual bool Read( const string& str , ImageTexture* tex )
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
