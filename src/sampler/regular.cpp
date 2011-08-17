/*
   FileName:      regular.cpp

   Created Time:  2011-08-11 19:40:05

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
 */

#include "regular.h"
#include "utility/assert.h"
#include <math.h>

// generate sample in one dimension
void RegularSampler::Generate1D( float* sample , unsigned num ) const
{
	Sort_Assert( sample != 0 );

	for( unsigned i = 0 ; i < num ; ++i )
		sample[i] = ( (float)i + 0.5f ) / (float)num ;
}

// generate sample in two dimension
void RegularSampler::Generate2D( float* sample , unsigned num ) const
{
	Sort_Assert( sample != 0 );
	
	unsigned n = (unsigned)sqrt((float)num);
	Sort_Assert( n * n == num );
	unsigned dn = 2 * n;

	for( unsigned i = 0 ; i < n ; ++i )
	{
		unsigned offset = dn * i;
		for( unsigned j = 0 ; j < dn ; j+=2 )
		{
			sample[offset+j] = ( (float)j/2 + 0.5f ) / (float)n ;
			sample[offset+j+1] = ( (float)i + 0.5f ) / (float)n ;
		}
	}
}

// round the size for sampler
unsigned RegularSampler::RoundSize( unsigned size ) const
{
	unsigned r = (unsigned)ceil( sqrt((float)size) );
	if( r == 0 ) r = 1;
	return r * r;
}
