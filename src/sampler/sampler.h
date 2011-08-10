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

// sample defination
class Sample
{
// public field
public:
	float				img_u , img_v;
	vector<unsigned>	dim0;
	vector<unsigned>	dim1;
	float**				sample_1d;
	float**				sample_2d;

	// default constructor
	Sample()
	{
		img_u = 0.0f;
		img_v = 0.0f;
		sample_1d = 0;
		sample_2d = 0;
	}
};

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

	// generate samples
	// para 'sample' : the sample array
	// para 'num'    : the number of samples to be generated
	virtual void GenerateSamples( Sample* sample , unsigned num );

	// round the size for sampler
	// para 'size' : number of samples to be generated
	// result      : the modified number
	// note : samples like jittered sampling needs to overwrite the method
	virtual unsigned RoundSize( unsigned size ) const { return size; }

// protected medthod
protected:
	// generate sample in one dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate1D( float* sample , int num ) = 0;

	// generate sample in two dimension
	// para 'sample' : the memory to save the sampled data
	// para 'num'    : the number of samples to be generated
	virtual void Generate2D( float* sample , int num ) = 0;
};

#endif