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

// include the header
#include "texture.h"
#include "spectrum/spectrum.h"
#include "utility/strhelper.h"

////////////////////////////////////////////////////////////
// definition of constant texture
class ConstantTexture : public Texture
{
public:
	DEFINE_CREATOR( ConstantTexture , Texture , "constant" );

	// default constructor
	ConstantTexture();
	// constructor from three float
	// para 'r' : r component
	// para 'g' : g component
	// para 'b' : b component
	ConstantTexture( float r , float g , float b );
	// destructor
	~ConstantTexture();

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const
	{
		return m_Color;
	}

	// get the texture value
	// para 'intersect' : the intersection
	// para 'result'    : the result
	virtual Spectrum Evaluate( const Intersection* intersect ) const
	{
		return m_Color;
	}

private:
	// the color for the texture
	Spectrum	m_Color;

	// initialize the data
	void _init();

	// register properties
	void _registerAllProperty();

// property handler
	class ColorProperty : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(ColorProperty,Texture);

		// set value
		void SetValue( const std::string& value )
		{
			ConstantTexture* ct = CAST_TARGET(ConstantTexture);
			ct->m_Color = SpectrumFromStr( value );
		}
	};
};
