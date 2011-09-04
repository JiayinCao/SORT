/*
   FileName:      skylight.h

   Created Time:  2011-08-04 17:41:30

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SKYLIGHT
#define	SORT_SKYLIGHT

#include "light.h"
#include "utility/creator.h"
#include "geometry/sky/sky.h"

//////////////////////////////////////////////////////////////////////////////////////
// definition of sky light
class	SkyLight : public Light
{
// public method
public:
	DEFINE_CREATOR(SkyLight);

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
	virtual Spectrum sample_l( const Intersection& intersect , const LightSample* ls , Vector& wi , float delta , float* pdf , Visibility& visibility ) const;

	// sample light density
	virtual Spectrum sample_l( const Intersection& intersect , const Vector& wo ) const;

	// total power of the light
	virtual Spectrum Power() const;

	// whether the light is a delta light
	virtual bool IsDelta() const { return false; }

	// get intersection between the light and the ray
	virtual bool Evaluate( const Ray& ray , Intersection* intersect , Spectrum& radiance ) const;

	// the pdf for specific sampled directioin
	// note : none-delta light must overwrite this method
	virtual float Pdf( const Point& p , const Vector& wi ) const;

	// sample a ray from light
	// para 'ls'       : light sample
	// para 'r'       : the light vector
	// para 'pdf'      : the properbility density function
	virtual void sample_l( const LightSample& ls , Ray& r , float* pdf ) const;

// private field
private:
	// the sky
	Sky*	sky;

	// initialize default value
	void _init();
	// release
	void _release();

	// register all properties
	void _registerAllProperty();

	class TypeProperty : public PropertyHandler<Light>
	{
	public:
		TypeProperty(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			SkyLight* light = CAST_TARGET(SkyLight);
			light->sky = CREATE_TYPE( str , Sky );
		}
	};
	class PropertyPasser : public PropertyHandler<Light>
	{
	public:
		PropertyPasser(Light* light):PropertyHandler(light){}
		void SetValue( const string& str )
		{
			SkyLight* light = CAST_TARGET(SkyLight);
			if( light->sky )
				light->sky->SetProperty( m_name , str );
			else
				LOG_WARNING<<"There is no sky attached to the light"<<ENDL;
		}
	};
};

#endif
