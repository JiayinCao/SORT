/*
 * filename :	texture.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header file
#include "texture.h"

// static value
TexShower* Texture::m_pTexShower = 0;

// default constructor
Texture::Texture()
{
	// inintialize default value
	_init();
}

// destructor
Texture::~Texture()
{
}

// initialize default value
void Texture::_init()
{
	m_pTexShower = 0;
}

// set the texture shower
void Texture::SetTexShower( TexShower* texShower )
{
	// delete previous texture shower
	SAFE_DELETE( m_pTexShower );

	// set new one
	m_pTexShower = texShower;
}

// output the texture
bool Texture::Output( const string& str )
{
	if( m_pTexShower )
		return m_pTexShower->Output( this , str );
	
	cout<<"there is no texture shower"<<endl;

	return false;
}
