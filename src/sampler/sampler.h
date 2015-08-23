/*
   FileName:      sampler.h

   Created Time:  2011-08-10 12:59:18

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SAMPLER
#define	SORT_SAMPLER

#include "sort.h"
#include <vector>
#include "sample.h"
#include "managers/memmanager.h"
#include <algorithm>
#include "utility/creator.h"

/////////////////////////////////////////////////////////////////////////////////
// definitation of the sampler
class	Sampler
{
// public method
public:
	// default constructor
	Sampler();
	// destructor
	virtual ~Sampler();

	// round the size for sampler
	// para 'size' : number of samples to be generated
	// result      : the modified number
	// note : samples like jittered sampling needs to overwrite the method
	virtual unsigned RoundSize( unsigned size ) const { return size; }

	// generate sample in one dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate1D( float* sample , unsigned num , bool accept_uniform = false ) const = 0;

	// generate sample in two dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate2D( float* sample , unsigned num , bool accept_uniform = false ) const = 0;
};

// generate shuffle index
inline const unsigned* ShuffleIndex( unsigned size )
{
	unsigned* shuffle = SORT_MALLOC_ARRAY( unsigned , size );
	for( unsigned i = 0 ; i < size ; i++ )
		shuffle[i] = i;
	std::random_shuffle( shuffle , shuffle + size );
	
	return shuffle;
}

#endif