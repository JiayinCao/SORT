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
#include "checkboxtexture.h"

IMPLEMENT_CREATOR( CheckBoxTexture );

// default constructor
CheckBoxTexture::CheckBoxTexture():
	m_Color0( 1.0f , 1.0f , 1.0f ),
	m_Color1( 0.0f , 0.0f , 0.0f )
{
	_init();
}

// constructor from two colors
CheckBoxTexture::CheckBoxTexture( const Spectrum& c0 , const Spectrum& c1 )
{
	m_Color0 = c0;
	m_Color1 = c1;

	_init();
}

// constructor from six float
CheckBoxTexture::CheckBoxTexture( 	float r0 , float g0 , float b0 , 
							float r1 , float g1 , float b1 ):
	m_Color0( r0 , g0 , b0 ) , m_Color1( r1 , g1 , b1 )
{
	_init();
}

// destructor
CheckBoxTexture::~CheckBoxTexture()
{
}

// overwrite init method
void CheckBoxTexture::_init()
{
	// by default , the width and height if not zero
	// because making width and height none zero costs nothing
	// while makeing them zero forbids showing the texture
	m_iTexWidth = 16;
	m_iTexHeight = 16;

	// register properties
	_registerAllProperty();
}

// get the color
Spectrum CheckBoxTexture::GetColor( int x , int y ) const 
{
	// filter the coorindate first
	_texCoordFilter( x , y );

	// return the color
	int delta_x = ( x - (int)m_iTexWidth / 2 );
	int delta_y = ( y - (int)m_iTexHeight / 2 );

	if( ( delta_x >= 0 && delta_y < 0 ) || ( delta_x < 0 && delta_y >= 0 ) )
		return m_Color1;
	else
		return m_Color0;
}

// register properties
void CheckBoxTexture::_registerAllProperty()
{
	_registerProperty( "color0" , new Color0Property( this ) );
	_registerProperty( "color1" , new Color1Property( this ) );
}

// set check box color
void CheckBoxTexture::SetCheckBoxColor( const Spectrum& c0 , const Spectrum& c1 )
{
	m_Color0 = c0;
	m_Color1 = c1;
}
