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

#pragma once

// include the header file
#include "compositetexture.h"

/////////////////////////////////////////////////////////////////////////
//	definition of render target
//	all of the generated image will be rendered to render target.
class	RenderTarget : public ComTexture
{
//public method
public:
	// constructor and destructor
	RenderTarget(){}
	~RenderTarget(){}

	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// para 'r' : red component
	// para 'g' : green component
	// para 'b' : blue component
	void SetColor( int x , int y , float r , float g , float b );

	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// para 'c' : color in spectrum form
	void SetColor( int x , int y , const Spectrum& c )
	{
		SetColor( x , y , c.GetR() , c.GetG() , c.GetB() );
	}
};
