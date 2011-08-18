/*
   FileName:      pointlight.h

   Created Time:  2011-08-04 12:48:35

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
	// para 'wi'		: input vector in world space
	// para 'delta'		: a delta to offset the original point
	// para 'pdf'		: property density function value of the input vector
	// para 'visibility': visibility tester
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const;

	// total power of the light
	virtual Spectrum Power() const
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
