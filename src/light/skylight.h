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
	DEFINE_CREATOR(SkyLight,"skylight");

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

	// total power of the light
	virtual Spectrum Power() const;

	// whether the light is a delta light
	virtual bool IsDelta() const { return false; }

	// the pdf for specific sampled directioin
	// note : none-delta light must overwrite this method
	virtual float Pdf( const Point& p , const Vector& wi ) const;

// private field
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
		void SetValue( const string& str )
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
		void SetValue( const string& str )
		{
			SkyLight* light = CAST_TARGET(SkyLight);
			if( light->sky )
				light->sky->SetProperty( m_name , str );
			else
				LOG_WARNING<<"There is no sky attached to the light"<<ENDL;
		}
	};
	class TransformProperty : public PropertyHandler<Light>
	{
	public:
		PH_CONSTRUCTOR(TransformProperty,Light);
		void SetValue( const string& str )
		{
			SkyLight* light = CAST_TARGET(SkyLight);
			light->transform = TransformFromStr(str);

			if( light->sky )
				light->sky->SetTransform( light->transform );
		}
	};
};

#endif
