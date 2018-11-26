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
#include "utility/creator.h"
#include "geometry/sky/sky.h"
#include "utility/log.h"

//////////////////////////////////////////////////////////////////////////////////////
// definition of sky light
class	SkyLight : public Light
{
public:
	DEFINE_CREATOR(SkyLight, Light, "skylight");

	// default constructor
	SkyLight(){_init();}
	// destructor
	~SkyLight(){_release();}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const;

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual Spectrum sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const;

	// sample light density
	virtual Spectrum Le( const Intersection& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const;

	// get intersection between the light and the ray
	virtual bool Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const;

	// whether the light is an infinite light
	virtual bool IsInfinite() const { return true; }

	// total power of the light
	virtual Spectrum Power() const;

	// whether the light is a delta light
	virtual bool IsDelta() const { return false; }

	// the pdf for specific sampled directioin
	// note : none-delta light must overwrite this method
	virtual float Pdf( const Point& p , const Vector& wi ) const;


private:
	// the sky
	Sky*	sky;

	// The transformation
	Transform	transform;

	// initialize default value
	void _init();
	// release
	void _release();

	// register all properties
	void _registerAllProperty();

	class TypeProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(TypeProperty,Light);
		void SetValue( const std::string& str )
		{
			SkyLight* light = CAST_TARGET(SkyLight);
			light->sky = CREATE_TYPE( str , Sky );

			light->sky->SetTransform( light->transform );
		}
	};
	class PropertyPasser : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(PropertyPasser,Light);
		void SetValue( const std::string& str )
		{
			SkyLight* light = CAST_TARGET(SkyLight);
			if( light->sky )
				light->sky->SetProperty( m_name , str );
			else
                slog( WARNING , LIGHT , "There is no sky attached to light" );
		}
	};
};
