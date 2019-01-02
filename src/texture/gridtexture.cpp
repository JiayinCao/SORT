/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "gridtexture.h"

IMPLEMENT_CREATOR( GridTexture );

// default constructor
GridTexture::GridTexture():
	m_Color0( 0.0f , 0.0f , 0.0f ),
	m_Color1( 1.0f , 1.0f , 1.0f )
{
	m_iTexWidth = m_iTexHeight = 16;
	m_Threshold = 0.9f;
}

// get the color
Spectrum GridTexture::GetColor( int x , int y ) const 
{
	// filter the coorindate first
	_texCoordFilter( x , y );

	// return the color
	int delta_x = ( x - (int)m_iTexWidth / 2 );
	int delta_y = ( y - (int)m_iTexHeight / 2 );

	// the size for the center grid
	float w_size = ( m_iTexWidth * 0.5f * m_Threshold );
	float h_size = ( m_iTexHeight * 0.5f * m_Threshold );

	if( delta_x <= w_size && delta_x > -w_size && 
		delta_y <= h_size && delta_y > -h_size )
		return m_Color1;
	else
		return m_Color0;
}

// set color of the spectrum
void GridTexture::SetGridColor( const Spectrum& c0 , const Spectrum& c1 )
{
	m_Color0 = c0;
	m_Color1 = c1;
}
