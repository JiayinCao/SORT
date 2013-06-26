/*
   FileName:      random.h

   Created Time:  2011-08-10 16:07:28

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_RANDOM
#define	SORT_RANDOM

#include "sampler.h"

////////////////////////////////////////////////////////////////////////////////////////////
// definition of random sampler
class RandomSampler : public Sampler
{
// public method
public:
	DEFINE_CREATOR( RandomSampler , "random" );

	// default constructor
	RandomSampler(){}
	// destructor
	~RandomSampler(){}

	// generate sample in one dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate1D( float* sample , unsigned num ) const;

	// generate sample in two dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate2D( float* sample , unsigned num ) const;
};

#endif