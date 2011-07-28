/*
 * filename :	pointlight.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_POINTLIGHT
#define	SORT_POINTLIGHT

// include the header
#include "light.h"
#include "utility/strhelper.h"

//////////////////////////////////////////////////////////////////////
// definition of point light
class PointLight : public Light
{
// public method
public:
	DEFINE_CREATOR( PointLight );

	// default constructor
	PointLight(){_registerAllProperty();}
	// destructor
	~PointLight(){}

	// sample ray from light
	// para 'intersect' : intersection information
	// para 'wo'		: output vector
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float* pdf ) const;

	// set transformation
	virtual void	SetTransform( const Transform& transform )
	{
		light2world = transform;
		pos = transform(Point(0.0f,0.0f,0.0f));
	}

	// total power of the light
	virtual Spectrum Power() const
	{return 4 * PI * intensity;}

// private field
private:
	// the light intensity
	Spectrum intensity;

	// position for light
	Point	pos;

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
			PointLight* light = CAST_TARGET(PointLight);
			light->intensity = SpectrumFromStr(str);
		}
	};
	class PosProperty : public PropertyHandler<Light>
	{
	public:
		PosProperty(Light* light):PropertyHandler(light){}

		// set value
		void SetValue( const string& str )
		{
			PointLight* light = CAST_TARGET(PointLight);
			light->pos = PointFromStr( str );
		}
	};
};

#endif