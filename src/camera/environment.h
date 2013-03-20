/*
   FileName:      environment.h

   Created Time:  2011-08-04 12:52:04

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
	EnvironmentCamera(){_init();}
	// destructor
	~EnvironmentCamera(){}

	// generate a ray given a pixel
	virtual Ray	GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const;

	// set transformation matrix
	void SetTransform( const Matrix& m ) { m_transform = m; }

// private field
private:
	// a rotation transformation
	Matrix	m_transform;
	
	// initialize data
	void _init();
	
	// register all properties
	void _registerAllProperty();
};

#endif
