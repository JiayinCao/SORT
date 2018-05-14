/*
   FileName:      skysphere.h

   Created Time:  2011-08-04 12:51:53

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SKYSPHERE
#define	SORT_SKYSPHERE

#include "sky.h"
#include "texture/imagetexture.h"

class Distribution2D;

////////////////////////////////////////////////////////////////////////
// definition of sky sphere
class	SkySphere : public Sky
{
// public method
public:
	DEFINE_CREATOR( SkySphere , Sky , "sky_sphere" );

	// default constructor
	SkySphere(){_init();}
	// destructor
	~SkySphere(){_release();}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Vector& r ) const;

	// get the average radiance
	virtual Spectrum GetAverage() const;

	// sample direction
	virtual Vector sample_v( float u , float v , float* pdf , float* area_pdf ) const;

	// get the pdf
	float Pdf( const Vector& wi ) const ;

// private field
private:
	ImageTexture	m_sky;
	Distribution2D*	distribution;

	// initialize default value
	void _init();
	// release
	void _release();
	// register property
	void _registerAllProperty();
	// generate 2d distribution
	void _generateDistribution2D();

// property handler
	class ImageProperty : public PropertyHandler<Sky>
	{
	public:
		PH_CONSTRUCTOR(ImageProperty,Sky);

		// set value
		void SetValue( const string& str )
		{
			SkySphere* sky = CAST_TARGET(SkySphere);
			sky->m_sky.LoadImageFromFile( str );
			sky->_generateDistribution2D();
		}
	};
};

#endif
