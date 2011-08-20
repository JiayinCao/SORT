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

////////////////////////////////////////////////////////////////////////
// definition of sky sphere
class	SkySphere : public Sky
{
// public method
public:
	DEFINE_CREATOR( SkySphere );

	// default constructor
	SkySphere();
	// destructor
	~SkySphere(){}

	// evaluate value from sky
	// para 'r' : the ray which misses all of the triangle in the scene
	// result   : the spectrum in the sky
	virtual Spectrum Evaluate( const Ray& r ) const;

// private field
private:
	ImageTexture m_sky;

	// register property
	void _registerAllProperty();

// property handler
	class ImageProperty : public PropertyHandler<Sky>
	{
	public:
		// constructor
		ImageProperty(Sky* sky):PropertyHandler(sky){}

		// set value
		void SetValue( const string& str )
		{
			SkySphere* sky = dynamic_cast<SkySphere*>(m_target);
			sky->m_sky.LoadImageFromFile( str );
		}
	};
};

#endif
