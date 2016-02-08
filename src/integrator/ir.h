/*
   FileName:      ir.h

   Created Time:  2016-2-6

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_IR
#define	SORT_IR

// include the header file
#include "integrator.h"
#include "geometry/intersection.h"

class Bsdf;

struct VirtualLightSource
{
	Intersection	intersect;
	Vector			wi;
	Spectrum		power;
	int				depth;
};

/////////////////////////////////////////////////////////////////////////////
// definition of Instant Radiosity
// note : Instant Radiosity is a subset of directional path tracing
//        It has two seperate passes. First pass generates virtual light
//        sources along the path tracing from light sources. Second pass
//        will use those virtual light source to evaluate indirect
//        illumination. Direct illumination is handled the same way in
//        directlight integrator.
class	InstantRadiosity : public Integrator
{
// public method
public:

	DEFINE_CREATOR( InstantRadiosity , "ir" );

	InstantRadiosity() {
		m_nLightPaths = 64;
		m_fMinDist = 1.0f;
		m_fMinSqrDist = 1.0f;
		m_nLightPathSet = 1;

		_registerAllProperty();
	}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;
    
	// Preprocess: In preprocessing stage, numbers of virtual light sources
    // are generated along the path tracing from light sources.
    virtual void PreProcess();
	// post-process after rendering
	virtual void PostProcess();

// private field
private:
	// light path set
	int		m_nLightPathSet;

    // number of paths to follow when creating virtual light sources
	int		m_nLightPaths;

	// distant threshold
	float	m_fMinDist;
	float	m_fMinSqrDist;

	// container for light sources
	list<VirtualLightSource>*	m_pVirtualLightSources;

	// register property
	void _registerAllProperty();

	// private method of li
	Spectrum _li( const Ray& ray , bool ignoreLe = false , float* first_intersect_dist = 0 ) const;

	class LightPathNumProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(LightPathNumProperty,Integrator);
		void SetValue( const string& str )
		{
			InstantRadiosity* ir = CAST_TARGET(InstantRadiosity);
			if( ir )
				ir->m_nLightPaths = atoi( str.c_str() );
		}
	};
	class MinDistanceProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(MinDistanceProperty,Integrator);
		void SetValue( const string& str )
		{
			InstantRadiosity* ir = CAST_TARGET(InstantRadiosity);
			if( ir )
			{
				ir->m_fMinDist = (float)atof( str.c_str() );
				ir->m_fMinSqrDist = ir->m_fMinDist * ir->m_fMinDist;
			}
		}
	};
	class LightPathSetProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(LightPathSetProperty,Integrator);
		void SetValue( const string& str )
		{
			InstantRadiosity* ir = CAST_TARGET(InstantRadiosity);
			if( ir )
				ir->m_nLightPathSet = (int)atof( str.c_str() );
		}
	};
};

#endif
