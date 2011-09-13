/*
   FileName:      bidirpath.h

   Created Time:  2011-09-03 10:18:33

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_BIDIRPATH
#define	SORT_BIDIRPATH

#include "integrator.h"
#include "geometry/point.h"
#include "geometry/vector.h"
#include "geometry/intersection.h"

class	Primitive;
class	Bsdf;

struct BDPT_Vertex
{
	Point		p;		// the position of the vertex
	Vector		n;		// the normal of the vertex
	Primitive*	pri;	// the primitive
	Vector		wi;		// in direction
	Vector		wo;		// out direction
	Bsdf*		bsdf;	// bsdf from material
	float		pdf;	// the pdf for the vertex

	Intersection	inter;	//temp

	BDPT_Vertex()
	{
		pri = 0;
		pdf = 1.0f;
		bsdf = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////
// definition of bidirectional path tracing
class BidirPathTracing : public Integrator
{
// public method
public:
	// default constructor
	BidirPathTracing( const Scene& s , unsigned ppp ):path_per_pixel(ppp),Integrator(s){}
	// destructor
	~BidirPathTracing(){}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;

	// request samples
	virtual void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num );

	// generate samples
	// para 'sampler' : the sampling method
	// para 'samples' : the samples to be generated
	// para 'ps'      : number of pixel sample to be generated
	// para 'scene'   : the scene to be rendered
	virtual void GenerateSample( const Sampler* sampler , PixelSample* samples , unsigned ps , const Scene& scene ) const;

	// output log information
	virtual void OutputLog() const;

// private field
private:
	unsigned path_per_pixel; // light sample per pixel sample per light

	// generate path
	unsigned	_generatePath( const Ray& ray , float base_pdf , vector<BDPT_Vertex>& path , unsigned max_vert ) const;
	// evaluate path
	// para 'epath' : path starting from eye point
	// para 'esize' : sub-path size ( we're not evaluating full path )
	// para 'lpath' : path starting from light
	// para 'lsize' : sub-path size ( we're not evaluating full path )
	Spectrum	_evaluatePath(	const vector<BDPT_Vertex>& epath , int esize , 
								const vector<BDPT_Vertex>& lpath , int lsize ) const;
	// compute G term
	Spectrum	_Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const;
};

#endif
