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

// include the file
#include "texture.h"
#include "utility/strhelper.h"

///////////////////////////////////////////////////////////////
// definition of grid texture
class	GridTexture : public Texture
{
public:
	DEFINE_CREATOR( GridTexture , Texture , "grid" );

	// default constructor
	GridTexture();
	// constructor from two colors
	// para 'c0' :	color0
	// para 'c1' :	color1
	GridTexture( const Spectrum& c0 , const Spectrum& c1 );
	// constructor from six float
	// para 'r0' :	r component of the first color
	// para 'g0' :	g component of the first color
	// para 'b0' :	b component of the first color
	// para 'r1' :	r component of the second color
	// para 'g1' :	g component of the second color
	// para 'b1' :	b component of the second color
	GridTexture( 	float r0 , float g0 , float b0 ,
					float r1 , float g1 , float b1 );
	// destructor
	~GridTexture();
	
	// get the texture value
	// para 'x'	:	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	virtual Spectrum GetColor( int x , int y ) const;

	// set color of the spectrum
	void	SetGridColor( const Spectrum& c0 , const Spectrum& c1 );

private:
	// two colors
	Spectrum	m_Color0;
	Spectrum	m_Color1;

	// the threshold for the center grid , default value is 0.9f
    float		m_Threshold;

	// init default values
	void	_init();

	// register properties
	void _registerAllProperty();

	// property handler
	class Color0Property : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(Color0Property,Texture);

		// set value
		void SetValue( const std::string& value )
		{
			GridTexture* ct = CAST_TARGET(GridTexture);
			ct->m_Color0 = SpectrumFromStr( value );
		}
	};
	class Color1Property : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(Color1Property,Texture);

		// set value
		void SetValue( const std::string& value )
		{
			GridTexture* ct = CAST_TARGET(GridTexture);
			ct->m_Color1 = SpectrumFromStr( value );
		}
	};
	class ThresholdProperty : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(ThresholdProperty,Texture);

		// set value
		void SetValue( const std::string& value )
		{
			GridTexture* ct = CAST_TARGET(GridTexture);
			ct->m_Threshold = (float)atof( value.c_str() );
		}
	};
};
