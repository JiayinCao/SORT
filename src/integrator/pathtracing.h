/*
   FileName:      pathtracing.h

   Created Time:  2011-08-31 09:34:34

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_PATHTRACING
#define	SORT_PATHTRACING

#include "integrator.h"

//////////////////////////////////////////////////////////////////////////////////////
//	definition of direct light
class	PathTracing : public Integrator
{
// public method
public:
	DEFINE_CREATOR( PathTracing , "pathtracing" );

	// default constructor
	PathTracing()
	{
		path_per_pixel = 3; // default maxium path is 3
		_registerAllProperty();	// register properties
	}
	// destructor
	~PathTracing(){}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;

	// request samples
	virtual void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num );

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const;

	// output log information
	virtual void OutputLog() const;

// private field
private:
	unsigned path_per_pixel; // light sample per pixel sample per light

	// register property
	void _registerAllProperty();

	class MaxPathProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(MaxPathProperty,Integrator);
		void SetValue( const string& str )
		{
			PathTracing* pathtracing = CAST_TARGET(PathTracing);

			if( pathtracing )
				pathtracing->path_per_pixel = atoi( str.c_str() );
		}
	};
};
#endif