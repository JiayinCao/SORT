/*
 * filename :	environment.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_ENVIRONMENT
#define	SORT_ENVIRONMENT

// include header file
#include "camera.h"
#include "geometry/matrix.h"

//////////////////////////////////////////////////////////////
// definition of environment camera
class	EnvironmentCamera : public Camera
{
// public method
public:
	// default constructor
	EnvironmentCamera(){}
	// destructor
	~EnvironmentCamera(){}

	// generate a ray given a pixel
	virtual Ray	GenerateRay( unsigned x , unsigned y ) const;

	// set transformation matrix
	void SetTransform( const Matrix& m ) { m_transform = m; }

// private field
private:
	// a rotation transformation
	Matrix	m_transform;
};

#endif
