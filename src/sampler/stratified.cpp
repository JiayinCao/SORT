/*
   FileName:      stratified.cpp

   Created Time:  2011-08-11 13:25:57

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "stratified.h"
#include "utility/sassert.h"
#include "utility/rand.h"
#include <math.h>

// generate sample in one dimension
void StratifiedSampler::Generate1D( float* sample , unsigned num ) const
{
	Sort_Assert( sample != 0 );

	for( unsigned i = 0 ; i < num ; ++i )
		sample[i] = ( (float)i + sort_canonical() ) / (float)num ;
}

// generate sample in two dimension
void StratifiedSampler::Generate2D( float* sample , unsigned num ) const
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
			sample[offset+j] = ( (float)j/2 + sort_canonical() ) / (float)n ;
			sample[offset+j+1] = ( (float)i + sort_canonical() ) / (float)n ;
		}
	}
}

// round the size for sampler
unsigned StratifiedSampler::RoundSize( unsigned size ) const
{
	unsigned r = (unsigned)ceil( sqrt((float)size) );
	return r * r;
}