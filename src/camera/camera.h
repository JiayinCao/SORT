/*
   FileName:      camera.h

   Created Time:  2011-08-04 12:51:59

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_CAMERA
#define	SORT_CAMERA

#include "geometry/point.h"
#include "geometry/ray.h"

// pre-decleration of render target
class RenderTarget;
class PixelSample;

////////////////////////////////////////////////////////////////////
//	definition of camera
class	Camera
{
// public method
public:
	// default constructor
	Camera(){ _init(); }
	// destructor
	virtual ~Camera(){}

	// generate a ray given a pixel
	virtual Ray	GenerateRay( float x , float y , const PixelSample& ps ) const = 0;

	// set a render target
	void SetRenderTarget( RenderTarget* rt ) { m_rt = rt; }

	// get and set eye point
	const Point& GetEye() const { return m_eye; }
	void SetEye( const Point& eye ) { m_eye = eye; }

// protected field
protected:
	// the eye point
	Point	m_eye;
	// the render target
	RenderTarget* m_rt;

// private method
	// initialize default data
	void _init() { m_rt = 0; }
};

#endif
