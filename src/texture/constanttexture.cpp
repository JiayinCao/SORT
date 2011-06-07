/*
 * filename :	constanttexture.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "constanttexture.h"

// default constructor
ConstantTexture::ConstantTexture()
{
	_init();
}

// constructor from three float
ConstantTexture::ConstantTexture( float r , float g , float b ):
	m_Color( r , g , b )
{
	_init();
}

// destructor
ConstantTexture::~ConstantTexture()
{
}

// overwrite init method
void ConstantTexture::_init()
{
	// by default , the width and height if not zero
	// because making width and height none zero costs nothing
	// while makeing them zero forbids showing the texture
	m_iTexWidth = 16;
	m_iTexHeight = 16;
}

