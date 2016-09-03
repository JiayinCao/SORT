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
#include "math/point.h"
#include "math/vector3.h"
#include "geometry/intersection.h"

class	Primitive;
class	Bsdf;

struct BDPT_Vertex
{
	Point			p;                  // the position of the vertex
	Vector			n;                  // the normal of the vertex
	Vector			wi;                 // in direction
	Vector			wo;                 // out direction
	Bsdf*			bsdf = nullptr;		// bsdf from material
	float			rr = 0.0f;          // russian roulette
	Spectrum		throughput;         // through put
	Intersection	inter;              // intersection

	// For further detail, please refer to the paper "Implementing Vertex Connection and Merging"
	// MIS factors
	float		vc = 0.0f;
	float		vcm = 0.0f;

	// depth of the vertex
    int			depth = 0;
};

struct Pending_Sample
{
	Vector2i	coord;
	Spectrum	radiance;
};

///////////////////////////////////////////////////////////////////////////////////
// definition of bidirectional path tracing
// BDPT is not finished yet, there are bugs in the following code.
// i'll try to finish it after i finish some more integrators.
class BidirPathTracing : public Integrator
{
// public method
public:

	DEFINE_CREATOR( BidirPathTracing , "bdpt" );

	// default constructor
	BidirPathTracing() {
		_registerProperty( "bdpt_mis" , new PTMISProperty(this) );
	}

	// return the radiance of a specific direction
	// para 'scene' : scene containing geometry data
	// para 'ray'   : ray with specific direction
	// result       : radiance along the ray from the scene<F3>
	virtual Spectrum	Li( const Ray& ray , const PixelSample& ps ) const;
    
    // request sample
    void RequestSample( Sampler* sampler , PixelSample* ps , unsigned ps_num );

	// support pending write
	virtual bool SupportPendingWrite() { return true; }

// private field
protected:
	bool	light_tracing_only = false;		// only do light tracing
	int		sample_per_pixel = 1;           // light sample per pixel

	// compute G term
	Spectrum	_Gterm( const BDPT_Vertex& p0 , const BDPT_Vertex& p1 ) const;

	// connect light sample
	Spectrum _ConnectLight(const BDPT_Vertex& eye_vertex, const Light* light ) const;
	
	// connect camera point
	void _ConnectCamera(const BDPT_Vertex& light_vertex , int len , const Light* light ) const;

	// connnect vertices
	Spectrum _ConnectVertices( const BDPT_Vertex& light_vertex , const BDPT_Vertex& eye_vertex , const Light* light ) const;

// private field
private:
	// use multiple importance sampling to sample direct illumination
	bool	m_bMIS = true;

	// Max Distance Property
	class PTMISProperty : public PropertyHandler<Integrator>
	{
	public:
		PH_CONSTRUCTOR(PTMISProperty,Integrator);
		void SetValue( const string& str )
		{
			BidirPathTracing* bdpt = CAST_TARGET(BidirPathTracing);
			if( bdpt )
				bdpt->m_bMIS = (atoi( str.c_str() )==1);
		}
	};

	// mis factor
	float MIS( float t ) const;
};

#endif
