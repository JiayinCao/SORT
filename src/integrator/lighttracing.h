/*
   FileName:      lighttracing.h

   Created Time:  2015-12-21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_LIGHTRACING
#define	SORT_LIGHTRACING

#include "bidirpath.h"

///////////////////////////////////////////////////////////////////////////////////
// definition of bidirectional path tracing
// BDPT is not finished yet, there are bugs in the following code.
// i'll try to finish it after i finish some more integrators.
class LightTracing : public BidirPathTracing
{
// public method
public:
	DEFINE_CREATOR(LightTracing, "lighttracing" );

	LightTracing() {
		light_tracing_only = true;
	}
};

#endif
