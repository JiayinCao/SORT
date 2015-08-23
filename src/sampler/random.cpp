/*
   FileName:      random.cpp

   Created Time:  2011-08-10 16:07:49

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "random.h"
#include "utility/sassert.h"
#include "utility/rand.h"

IMPLEMENT_CREATOR( RandomSampler );

// generate sample in one dimension
void RandomSampler::Generate1D( float* sample , unsigned num , bool accept_uniform ) const
{
	Sort_Assert( sample != 0 );

	for( unsigned i = 0 ; i < num ; ++i )
		sample[i] = sort_canonical();
}

// generate sample in two dimension
void RandomSampler::Generate2D( float* sample , unsigned num , bool accept_uniform ) const
{
	Sort_Assert( sample != 0 );

	int count = 2 * num;
	for( int i = 0 ; i < count ; i += 2 )
	{
		sample[i] = sort_canonical();
		sample[i+1] = sort_canonical();
	}
}