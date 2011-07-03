/*
 * filename :	environment.cpp
 *
 * programmer :	Cao Jiayin
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
