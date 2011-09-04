/*
   FileName:      bidirpath.cpp

   Created Time:  2011-09-03 10:18:18

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "bidirpath.h"

// return the radiance of a specific direction
Spectrum BidirPathTracing::Li( const Scene& scene , const Ray& ray , const PixelSample& ps ) const
{
	return 0.0f;
}

// request samples
void BidirPathTracing::RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num )
{
}

// generate samples
void BidirPathTracing::GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const
{
}

// output log information
void BidirPathTracing::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Bidirectional Path Tracing."<<ENDL;
	LOG<<"It supports all of the features in direct lighting algorithm."<<ENDL;
	LOG<<"Some global illumination effect is also supported in path tracing."<<ENDL;
	LOG<<"While it requires much more samples to reduce the noise to an acceptable level."<<ENDL;
	LOG<<"Bidirectional Path Tracing trace rays from eye and light source at the same time."<<ENDL;
}
