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

#include "light.h"
#include "utility/strhelper.h"
#include "math/vector3.h"

///////////////////////////////////////////////////////////////////////
// definition of spot light
class SpotLight : public Light
{
public:
	DEFINE_CREATOR( SpotLight , Light, "spot" );

	// default constructor
	SpotLight(){_registerAllProperty();}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const ;

	// total power of the light
	virtual Spectrum Power() const
	{return 4 * PI * intensity * ( 1.0f - 0.5f * ( cos_falloff_start + cos_total_range ) ) ;}

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual Spectrum sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const;

private:
    float	cos_falloff_start = Radians( 25.0f );
    float	cos_total_range = Radians( 30.0f );

	// register property
	void _registerAllProperty();

	class FalloffStartProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(FalloffStartProperty,Light);
		void SetValue( const std::string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
			light->cos_falloff_start = (float)cos( Radians( (float)atof( str.c_str() ) ) );
		}
	};
	class RangeProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(RangeProperty,Light);
		void SetValue( const std::string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
            light->cos_total_range = (float)cos( Radians( (float)atof( str.c_str() ) ) );
		}
	};
};
