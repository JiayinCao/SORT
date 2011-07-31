/*
 * filename :	light.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_LIGHT
#define	SORT_LIGHT

// include header
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "geometry/transform.h"
#include "utility/creator.h"
#include "utility/strhelper.h"
#include "geometry/scene.h"

// pre-decleration
class Intersection;
class Vector;

class Visibility
{
// public method
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
// definition of ligth
class	Light : public PropertySet<Light>
{
// public method
public:
	// default constructor
	Light(){}
	// destructor
	virtual ~Light(){}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wo'		: output vector
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float* pdf , Visibility& visibility ) const = 0;

	// set transformation
	virtual void	SetTransform( const Transform& transform )
	{light2world = transform;}

	// total power of the light
	virtual Spectrum Power( const Scene& ) const = 0;

// protected field
protected:
	// intensity for the light
	Spectrum	intensity;
	// transformation of the light
	Transform	light2world;

	// register property
	void _registerAllProperty()
	{
		_registerProperty( "intensity" , new IntensityProperty(this) );
	}

	// property handler
	class IntensityProperty : public PropertyHandler<Light>
	{
	public:
		// constructor
		IntensityProperty(Light* light):PropertyHandler(light){}

		// set value
		void SetValue( const string& str )
		{
			Light* light = CAST_TARGET(Light);
			light->intensity = SpectrumFromStr(str);
		}
	};
};

#endif
