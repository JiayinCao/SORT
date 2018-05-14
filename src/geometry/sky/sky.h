/*
   FileName:      sky.h

   Created Time:  2011-08-04 12:51:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SKY
#define	SORT_SKY

// include the header
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "math/vector3.h"
#include "math/transform.h"

class Point;

//////////////////////////////////////////////////////////////////
//	definition of sky
//	note: we could also use a very large box or sphere to replace
//		  the sky we need , while that would make some accelerator
//		  such as uniform grid unefficient. So we use a isolated
//		  sky to show the environment.
class	Sky : public PropertySet<Sky>
{
// public method
public:
	// default constructor
	Sky(){}
	// destructor
	virtual ~Sky(){}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Vector& r ) const = 0;

	// get the average radiance
	virtual Spectrum GetAverage() const = 0;

	// sample direction
	virtual Vector sample_v( float u , float v , float* pdf , float* area_pdf ) const = 0;

	// get the pdf
	virtual float Pdf( const Vector& wi ) const = 0;

	// setup transformation
	void SetTransform( const Transform& tf ){
		m_transform = tf;
	}

protected:
	Transform m_transform;
};

#endif
