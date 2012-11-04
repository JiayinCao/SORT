/*
   FileName:      dofperspective.h

   Created Time:  2011-08-10 19:40:28

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_DOFPERSPECTIVE
#define	SORT_DOFPERSPECTIVE

#include "camera/perspective.h"

///////////////////////////////////////////////////////////////////////////
// definition of dof_camera
// with multiple sampling method , the camera provides depth of field.
class DofPerspective : public PerspectiveCamera
{
// public method
public:
	// default constructor
	DofPerspective(){ lensRadius = 1.0f; focalDistance = 100.0f; }
	// destructor
	~DofPerspective(){}

	// generate ray
	virtual Ray GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const;

	// set len
	void SetLen( float len )
	{ lensRadius = len; }

	// set focal plane
	void SetFocalDistance( float distance )
	{ focalDistance = distance; }

// private field
private:
	float lensRadius;
	float focalDistance;
};

#endif
