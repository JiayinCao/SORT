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

	InstantRadiosity() {}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;

    // Preprocess: In preprocessing stage, numbers of virtual light sources
    // are generated along the path tracing from light sources.
    virtual void Preprocess();
    
// private field
private:
    
};

#endif
