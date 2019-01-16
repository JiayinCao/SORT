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

#pragma once

#include "core/define.h"
#include "spectrum/spectrum.h"

// texture filter
enum TEXCOORDFILTER{
	TCF_WARP = 0 ,
	TCF_CLAMP ,
	TCF_MIRROR
};

// WARNING : Code in this folder is easily several years old.
// 			 It is just functional for now. Will re-implement everything later.
//		 	 Priority is very low.

// pre-declare class
class Intersection;

//////////////////////////////////////////////////////////////
// definition of class Texture
class Texture
{
public:
	// default constructor
	Texture();
	Texture( int w , int h ) : m_iTexWidth(w) , m_iTexHeight(h) , m_TexCoordFilter(TCF_WARP) {
	}
	// destructor
	virtual ~Texture();

	// output the texture
	// para 'str' :	the name of the output entity
	// result : return true , if outputing is successful
	bool Output( const std::string& str );

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const = 0;

	// get color from uv coordinate
	virtual Spectrum GetColorFromUV( float u , float v ) const;

	// get the texture value
	// para 'intersect' : the intersection
	// result :	the spectrum value
	virtual Spectrum Evaluate( const Intersection* intersect ) const;

	// set texture coordinate filter mode
	void	SetTexCoordFilter( TEXCOORDFILTER mode );

	// get the size of the texture
	unsigned	GetWidth() const{
		return m_iTexWidth;
	}
	unsigned	GetHeight() const{
		return m_iTexHeight;
	}
	// whether the texture is valid
	virtual bool IsValid() {
		return true;
	}

protected:
	// the size of the texture
	// default values are zero
	unsigned	m_iTexWidth;
	unsigned	m_iTexHeight;

	// texture coordinate filter mode
	// default value is warp
	TEXCOORDFILTER	m_TexCoordFilter;

	// initialize default value for texture
	void _init();

	// do texture filter
	void _texCoordFilter( int& u , int&v ) const;
};
