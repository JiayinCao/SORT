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

// include the header file
#include "constanttexture.h"

IMPLEMENT_CREATOR( ConstantTexture );

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

	_registerAllProperty();
}

// register properties
void ConstantTexture::_registerAllProperty()
{
	_registerProperty( "color" , new ColorProperty( this ) );
}
