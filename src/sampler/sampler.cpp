/*
   FileName:      sampler.cpp

   Created Time:  2011-08-10 12:59:15

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "sampler.h"
#include "utility/assert.h"
#include "managers/memmanager.h"

// default constructor
Sampler::Sampler()
{
}

// destructor
Sampler::~Sampler()
{
}

// generate samples
void Sampler::GenerateSamples( Sample* sample , unsigned num )
{
	float* data = SORT_MALLOC_ARRAY( float , num * 2 )();
	Generate2D( data , num );

	for( unsigned k = 0 ; k < num ; k++ )
	{
		sample[k].img_u = data[2*k];
		sample[k].img_v = data[2*k+1];
	}

	Sort_Assert( sample );
	for( unsigned k = 0 ; k < num ; ++k )
	{
		for( unsigned i = 0 ; i < sample[k].dim0.size() ; ++i )
		{
			Sort_Assert( sample[k].sample_1d != 0 );
			Generate1D( sample[k].sample_1d[i] , sample[k].dim0[i] );
		}
		for( unsigned i = 0 ; i < sample[k].dim1.size() ; ++i )
		{
			Sort_Assert( sample[k].sample_2d != 0 );
			Generate2D( sample[k].sample_2d[i] , sample[k].dim1[i] );
		}
	}
}