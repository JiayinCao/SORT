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

// include header
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "math/transform.h"
#include "utility/creator.h"
#include "utility/strhelper.h"
#include "geometry/scene.h"
#include "math/vector3.h"

class Intersection;
class LightSample;
class Shape;

class Visibility
{
public:
	// default constructor
	Visibility( const Scene& s ):scene(s){}
	// destructor
	~Visibility(){}

	// whether it's visible from the light source
	bool	IsVisible() const
	{
		return !scene.GetIntersect( ray , 0 );
	}

// public field
	// the shadow ray
	Ray	ray;
	// the scene
	const Scene& scene;
};

////////////////////////////////////////////////////////////////////////
// definition of light
class	Light : public PropertySet<Light>
{
public:
    // constructor
    Light() {_registerAllProperty();}
	// destructor
	virtual ~Light(){scene=0;}

	// setup scene
	void	SetupScene( const Scene* s ) {scene=s;}
	
	// set transformation
	virtual void	SetTransform( const Transform& transform ) {light2world = transform;}

	// total power of the light
	virtual Spectrum Power() const = 0;

	// note : the following methods must be overwritten in non-delta light
	// whether the light is a delta light
	virtual bool IsDelta() const { return true; }

	// whether the light is an infinite light
	virtual bool IsInfinite() const { return false; }

	// get the shape of light
	virtual Shape* GetShape() const { return 0; }

	// the pdf for specific sampled directioin
	virtual float Pdf( const Point& p , const Vector& wi ) const { return 1.0f; }

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: properbility density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& dirToLight , float* distance , float* pdfw , float* emissionPdf , float* cosAtLight , Visibility& visibility ) const = 0;

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual Spectrum sample_l( const LightSample& ls , Ray& r , float* pdfW , float* pdfA , float* cosAtLight ) const = 0;

	// sample light density
	virtual Spectrum Le( const Intersection& intersect , const Vector& wo , float* directPdfA , float* emissionPdf ) const { return 0.0f; }

	// get intersection between the light and the ray
	virtual bool Le( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const { return false; }

	// set pdf of picking the light
	void SetPickPDF( float pdf ) {
		pickProp = pdf;
	}
	float PickPDF() const {
		return pickProp;
	}

protected:
	// scene containing the light
    const Scene* scene = nullptr;
	// intensity for the light
	Spectrum	intensity;
	// transformation of the light
	Transform	light2world;

	// pdf of picking the light
	float		pickProp;

	// register property
	void _registerAllProperty()
	{
		_registerProperty( "intensity" , new IntensityProperty(this) );
        _registerProperty( "transform" , new TransformProperty(this) );
	}

	// property handler
	class IntensityProperty : public PropertyHandler<Light>
	{
	public:
		// constructor
		PH_CONSTRUCTOR(IntensityProperty,Light);

		// set value
		void SetValue( const std::string& str )
		{
			Light* light = CAST_TARGET(Light);
			light->_setIntensity( SpectrumFromStr(str) );
		}
	};
    
    class TransformProperty : public PropertyHandler<Light>
    {
    public:
        PH_CONSTRUCTOR(TransformProperty,Light);
        void SetValue( const std::string& str )
        {
            Light* light = CAST_TARGET(Light);
            light->SetTransform( TransformFromStr(str) );
        }
    };
    
	// set light intensity
	virtual void _setIntensity( const Spectrum& e )
	{ intensity = e; }
};
