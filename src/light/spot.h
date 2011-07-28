/*
 * filename :	spot.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_SPOT
#define	SORT_SPOT

#include "light.h"
#include "utility/strhelper.h"

///////////////////////////////////////////////////////////////////////
// definition of spot light
class SpotLight : public Light
{
// public method
public:
	DEFINE_CREATOR( SpotLight );

	// default constructor
	SpotLight(){_registerAllProperty();}
	// destructor
	~SpotLight(){}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wo'		: output vector
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float* pdf ) const;

	// total power of the light
	virtual Spectrum Power() const
	{return 4 * PI * intensity * ( 1.0f - 0.5f * ( cos_falloff_start + cos_total_range ) ) ;}

// private field
private:
	Spectrum intensity;

	float	cos_falloff_start;
	float	cos_total_range;

	// initialize default value
	void _init();

	// register property
	void _registerAllProperty();

	// property handler
	class IntensityProperty : public PropertyHandler<Light>
	{
	public:
		// constructor
		IntensityProperty(Light* light):PropertyHandler(light){}

		// set value
		void SetValue( const string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
			light->intensity = SpectrumFromStr(str);
		}
	};
	class FalloffStartProperty : public PropertyHandler<Light>
	{
	public:
		FalloffStartProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
			light->cos_falloff_start = (float)cos(atof( str.c_str() ));
		}
	};
	class RangeProperty : public PropertyHandler<Light>
	{
	public:
		RangeProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
			light->cos_total_range = (float)cos(atof( str.c_str() ));
		}
	};
};

#endif