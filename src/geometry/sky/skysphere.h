/*
 * filename :	skysphere.h
 *
 * programmer :	Cao Jiayin
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
