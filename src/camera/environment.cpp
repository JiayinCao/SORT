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
#include "utility/sassert.h"
#include "utility/define.h"
#include "texture/rendertarget.h"
#include <math.h>
#include "sampler/sample.h"

// initialize data
void EnvironmentCamera::_init()
{
	m_type = CT_ENVIRONMENT;
	
	_registerAllProperty();
}

// generate a ray given a pixel
Ray	EnvironmentCamera::GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const
{
	Sort_Assert( m_rt != 0 );

	x += ps.img_u;
	y += ps.img_v;

	// generate ray
	float theta = PI * y / m_rt->GetHeight();
	float phi = 2 * PI * x / m_rt->GetWidth();
	Vector dir( sinf( theta ) * cosf( phi ) , cosf( theta ) , sinf( theta ) * sinf( phi ) );
	Ray r( m_eye , dir );

	// transform the ray
	r = m_transform(r);

	return r;
}

// register all properties
void EnvironmentCamera::_registerAllProperty()
{
	_registerProperty( "eye" , new EyeProperty( this ) );
}