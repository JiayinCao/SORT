/*
 * filename :	texture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_TEXTURE
#define SORT_TEXTURE

#include "../sort.h"
#include "texshower/texshower.h"

//////////////////////////////////////////////////////////////
// defination of class Texture
class Texture
{
// public method
public:
	// default constructor
	Texture();
	// destructor
	~Texture();

	// set tex shower
	// para 'texShower' : the pointer to the texshower to set
	static void SetTexShower( TexShower* texShower );
	// output the texture
	// result : return true , if outputing is successful
	bool Output( const string& str );

// private field
private:
	// the texture shower
	static TexShower*  m_pTexShower;

// private method
private:
	// initialize default value for texture
	void _init();
};

#endif
