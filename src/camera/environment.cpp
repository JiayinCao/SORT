/*
   FileName:      environment.cpp

   Created Time:  2011-08-04 12:52:01

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "environment.h"
#include "geometry/vector.h"
#include "geometry/ray.h"
#include "managers/logmanager.h"
#include "utility/define.h"
#include "texture/rendertarget.h"
#include <math.h>

// generate a ray given a pixel
Ray	EnvironmentCamera::GenerateRay( unsigned x , unsigned y ) const
{
	if( m_rt == 0 )
		LOG_ERROR<<"There is no render target bind to environment camera."<<CRASH;

	// generate ray
	float theta = PI * y / m_rt->GetHeight();
	float phi = 2 * PI * x / m_rt->GetWidth();
	Vector dir( sinf( theta ) * cosf( phi ) , cosf( theta ) , sinf( theta ) * sinf( phi ) );
	Ray r( m_eye , dir );

	// transform the ray
	r = m_transform(r);

	return r;
}
