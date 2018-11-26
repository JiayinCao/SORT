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

#include "sky.h"
#include "texture/imagetexture.h"

class Distribution2D;

////////////////////////////////////////////////////////////////////////
// definition of sky sphere
class	SkySphere : public Sky
{
public:
	DEFINE_CREATOR( SkySphere , Sky , "sky_sphere" );

	// default constructor
	SkySphere(){_init();}
	// destructor
	~SkySphere(){_release();}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Vector& r ) const;

	// get the average radiance
	virtual Spectrum GetAverage() const;

	// sample direction
	virtual Vector sample_v( float u , float v , float* pdf , float* area_pdf ) const;

	// get the pdf
	float Pdf( const Vector& wi ) const ;

private:
	ImageTexture	m_sky;
	Distribution2D*	distribution;

	// initialize default value
	void _init();
	// release
	void _release();
	// register property
	void _registerAllProperty();
	// generate 2d distribution
	void _generateDistribution2D();

// property handler
	class ImageProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(ImageProperty,Sky);

		// set value
		void SetValue( const std::string& str )
		{
			SkySphere* sky = CAST_TARGET(SkySphere);
			sky->m_sky.LoadImageFromFile( str );
			sky->_generateDistribution2D();
		}
	};
};
