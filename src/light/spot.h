/*
   FileName:      spot.h

   Created Time:  2011-08-04 12:48:43

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SPOT
#define	SORT_SPOT

#include "light.h"
#include "utility/strhelper.h"
#include "geometry/vector.h"

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
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , Vector& wi , float delta , float* pdf , Visibility& visibility ) const ;

	// total power of the light
	virtual Spectrum Power() const
	{return 4 * PI * intensity * ( 1.0f - 0.5f * ( cos_falloff_start + cos_total_range ) ) ;}

// private field
private:
	float	cos_falloff_start;
	float	cos_total_range;

	// initialize default value
	void _init();

	// register property
	void _registerAllProperty();

	class PosProperty : public PropertyHandler<Light>
	{
	public:
		PosProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
			Point p = PointFromStr( str );
			light->light2world.matrix.m[3] = p.x;
			light->light2world.matrix.m[7] = p.y;
			light->light2world.matrix.m[11] = p.z;
			light->light2world.invMatrix.m[3] = -p.x;
			light->light2world.invMatrix.m[7] = -p.y;
			light->light2world.invMatrix.m[11] = -p.z;
		}
	};
	class DirProperty : public PropertyHandler<Light>
	{
	public:
		DirProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			SpotLight* light = CAST_TARGET(SpotLight);
			Vector dir = Normalize(VectorFromStr( str ));
			Vector t0 , t1;
			CoordinateSystem( dir, t0 , t1 );
			Matrix& m = light->light2world.matrix;
			Matrix& inv = light->light2world.invMatrix;
			m.m[0] = t0.x; m.m[1] = dir.x; m.m[2] = t1.x;
			m.m[4] = t0.y; m.m[5] = dir.y; m.m[6] = t1.y;
			m.m[8] = t0.z; m.m[9] = dir.z; m.m[10] = t1.z;
			inv.m[0] = t0.x; inv.m[1] = t0.y; inv.m[2] = t0.z;
			inv.m[4] = dir.x; inv.m[5] = dir.y; inv.m[6] = dir.z;
			inv.m[8] = t1.x; inv.m[9] = t1.y; inv.m[10] = t1.z;
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
