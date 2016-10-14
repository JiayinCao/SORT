/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include header file
#include "uvtexture.h"
#include "geometry/intersection.h"

IMPLEMENT_CREATOR( UVTexture );

// get the texture value
Spectrum UVTexture::GetColor( int x , int y ) const
{
	// filter the x y coordinate
	_texCoordFilter( x , y );

	float u = ((float)x) / m_iTexWidth;
	float v = ((float)y) / m_iTexHeight;

	return Spectrum( u , v , 0.0f );
}

// get the texture value
// para 'intersect' : the intersection
// result :	the spectrum value
Spectrum UVTexture::Evaluate( const Intersection* intersect ) const
{
	if( intersect != 0 )
		return Spectrum( intersect->u , intersect->v , 0 );

	return Spectrum();
}

// initialize default data
void UVTexture::_init()
{
	m_iTexWidth = 16;
	m_iTexHeight = 16;
}
