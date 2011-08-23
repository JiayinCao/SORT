/*
   FileName:      sample.h

   Created Time:  2011-08-15 12:37:32

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SAMPLE
#define	SORT_SAMPLE

#include "sort.h"
#include <vector>
#include "utility/rand.h"

// Light Sample
class	LightSample
{
// public field
public:
	// id of the light to sample
	unsigned	light_id;
	// the sample parameters
	float		u , v;	// 2d sample data
	float		reserved;	// for memory alignment

	// default constructor
	LightSample(){}
};

// Bsdf Sample
class	BsdfSample
{
// public field
public:
	float	t;		// a canonical number to select bxdf from bsdf
	float	u , v;	// 2d sample data

	float	preserved;	// a preserved data for memory alignment

	// default constructor
	BsdfSample(bool auto_generate=false)
	{
		if( auto_generate )
		{
			t = sort_canonical();
			u = sort_canonical();
			v = sort_canonical();
		}
	}
};

// sample defination
class PixelSample
{
// public field
public:
	float					img_u , img_v;	// the range of the float2 should be (0,0) <-> (1,1)
	float					dof_u , dof_v;	// the range of the float2 should be (-1,-1) <-> (1,1)
	vector<LightSample*>	light_sample;
	vector<BsdfSample*>		bsdf_sample;

	// default constructor
	PixelSample()
	{
		img_u = 0.0f;
		img_v = 0.0f;
	}
};

#endif
