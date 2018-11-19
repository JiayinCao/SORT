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

// include texture
#include "texture.h"

/////////////////////////////////////////////////////////////////////////////////
//	definition of composite texture
class	ComTexture : public Texture
{
// public function
public:
	// constructor 
	ComTexture() { _init(); }
	// constructor from a texture
	// para 'tex' : texture to copy
	ComTexture( const Texture& tex ) { _init(); _copy( tex ); }
	// constructor from a comtexture
	// para 'tex' : texture to copy
	ComTexture( const ComTexture& tex ) { _init(); _copy( tex ); }
	// constructor for texture to construct a comtexture
	// para 'data' : the memory to the image data
	// para 'w'    : the width for the texture
	// para 'h'    : the height for the texture
	ComTexture( Spectrum* data , unsigned w , unsigned h ) 
	{
		m_pData = data ;
		m_iTexWidth = w;
		m_iTexHeight = h;
	}
	// destructor
	virtual ~ComTexture() { Release(); }

	// release the memory
	virtual void Release();

	// get color from render target
	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// result   : the corresponding color
	// note     : the x or y is out of range , use filter to make them in the range.
	//            if there is no data in the render target, there will be a run-time
	//			  and log LOG_ERROR.
	virtual Spectrum GetColor( int x , int y ) const;

	// set the size for the render target
	// para 'w' : width of the render target
	// para 'y' : height of the render target
	// note     : if any of the two edge is zero length , there will be a LOG_WARNING in
	//			  log. any call to the method will cause allocating memory if parameters
	//			  are both valid.
	virtual void SetSize( unsigned w , unsigned h );

	// = operator
	ComTexture& operator = ( const Texture& tex ) { _copy( tex ); return *this;}
	ComTexture& operator = ( const ComTexture& tex ) { _copy( tex ); return *this; }


protected:
	// the texture data
	Spectrum*	m_pData;

// private method
	// initialize default data
	void	_init();
	// copy data from a texture
	void	_copy( const Texture& tex );
	void	_copy( const ComTexture& tex );
};