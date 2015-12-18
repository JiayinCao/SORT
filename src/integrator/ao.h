/*
   FileName:      ao.h

   Created Time:  2015-11-16

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_AO
#define	SORT_AO

// include the header file
#include "integrator.h"

/////////////////////////////////////////////////////////////////////////////
// definition of whittedrt
// note : Whitted ray tracer only takes direct light into consideration,
//		  there are also specular reflection and refraction. While indirect 
//		  light, like color bleeding , is not supported.
class	AmbientOcclusion : public Integrator
{
// public method
public:

	DEFINE_CREATOR( AmbientOcclusion , "ao" );

	AmbientOcclusion() { maxDistance = 10.0f; }

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;

	// output log information
	virtual void OutputLog() const;

// private field
private:
	float	maxDistance;

	void _registerAllProperty();

	// Max Distance Property
	class MaxDistanceProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(MaxDistanceProperty,Integrator);
		void SetValue( const string& str )
		{
			AmbientOcclusion* ao = CAST_TARGET(AmbientOcclusion);
			if( ao )
				ao->maxDistance = (float)atof( str.c_str() );
		}
	};
};

#endif
