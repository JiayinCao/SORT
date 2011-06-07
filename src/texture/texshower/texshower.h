/*
 * filename :	texshower.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_TEXSHOWER
#define SORT_TEXSHOWER

// include the header file
#include "../../sort.h"
#include "../texture.h"

// pre-declare Texture class
class Texture;

////////////////////////////////////////////////////////////////////////////
// defination of TexShower
// TexShower is an abstract class that is responsible for outputing texture
// in different ways , such as saving the texture as an image file or 
// poping an window showing the texture.
class	TexShower
{
// public method
public:
	// default constructor
	TexShower() {}
	// destructor
	~TexShower() {}

	// output the file in different ways
	// para 'tex': the texture for outputing
	// para 'str': the name of the bmp file
	// result    : 'true' if outputing is successed
	virtual bool Output( const Texture* tex , const string& str ) = 0;
};

#endif
