/*
   FileName:      light.h

   Created Time:  2011-08-04 12:48:28

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const = 0;

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
