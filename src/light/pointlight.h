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
	virtual Spectrum sample_f( const Intersection& intersect , Vector& wi , float* pdf , Visibility& visibility ) const;

	// total power of the light
	virtual Spectrum Power( const Scene& ) const
	{return 4 * PI * intensity;}

// private field
private:
	// register property
	void _registerAllProperty();

	class PosProperty : public PropertyHandler<Light>
	{
	public:
		PosProperty(Light* light):PropertyHandler(light){}

		// set value
		void SetValue( const string& str )
		{
			PointLight* light = CAST_TARGET(PointLight);
			Point p = PointFromStr( str );
			light->light2world.matrix.m[3] = p.x;
			light->light2world.matrix.m[7] = p.y;
			light->light2world.matrix.m[11] = p.z;
			light->light2world.invMatrix.m[3] = -p.x;
			light->light2world.invMatrix.m[7] = -p.y;
			light->light2world.invMatrix.m[11] = -p.z;
		}
	};
};

#endif
