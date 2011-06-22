/*
 * filename :	camera.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_CAMERA
#define	SORT_CAMERA

#include "geometry/point.h"
#include "geometry/ray.h"

// pre-decleration of render target
class RenderTarget;

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
	virtual Ray	GenerateRay( unsigned x , unsigned y ) const = 0;

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
