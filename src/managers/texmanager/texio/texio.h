/*
 * filename :	texio.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_TEXIO
#define SORT_TEXIO

// texture output type
enum TEX_OUTPUT_TYPE
{
	TOT_BMP = 0,
	TOT_WINDOW ,
	TOT_END ,
};

// include the header file
#include "../../../sort.h"
#include "../../../texture/texture.h"

// pre-declare texture class
class Texture;

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
	TexIO(){tot_type=TOT_END;}
	// constructor from a type
	// para 'tot' : texture output type
	// note	      :	actually set tot_type in TexIO gains nothing
	TexIO( TEX_OUTPUT_TYPE tot ) { tot_type = tot; }
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
	// result     :	'true' if the input file is parsed successfully
	// note       : it's not a pure function because some shower doesn't need a reading func
	virtual bool Read( const string& str , const Texture* tex )
	{
		return false;
	}

	// get tot type
	TEX_OUTPUT_TYPE GetTOT()
	{
		return tot_type;
	}

// protected data
protected:
	// the type of the outputer
	TEX_OUTPUT_TYPE tot_type;
};

#endif
