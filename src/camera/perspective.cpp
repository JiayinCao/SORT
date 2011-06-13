/*
 * filename :	perspective.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "perspective.h"
#include "../utility/error.h"
#include "../texture/rendertarget.h"
#include <math.h>

// generate ray
Ray	PerspectiveCamera::GenerateRay( unsigned x , unsigned y ) const
{
	// check if there is render target
	if( m_rt == 0 )
		SCrash( "There is no render target set in the camera, can't get width and height of the image." );

	float w = (float)m_rt->GetWidth();
	float h = (float)m_rt->GetHeight();
	float aspect = w / h;

	float yScale = 1.0f / tan( m_fov / 2 );
	float xScale = yScale / aspect;

	Vector v;
	v.x = ( ( ( 2.0f * x ) / w ) - 1 ) / 2.0f / xScale ;
	v.y = -1.0f * ( ( ( 2.0f * ( h - y - 1 ) / h ) - 1 ) ) / 2.0f / yScale;
	v.z = 1.0f;

	Vector zaxis = ( m_target - m_eye ).Normalize();
	Vector xaxis = Cross( m_up , zaxis ).Normalize();
	Vector yaxis = Cross( zaxis , xaxis );

	Ray r;
	r.m_Dir.x = v.x * xaxis.x + v.y * yaxis.x + v.z * zaxis.x ;
	r.m_Dir.y = v.x * xaxis.y + v.y * yaxis.y + v.z * zaxis.y ;
	r.m_Dir.z = v.x * xaxis.z + v.y * yaxis.z + v.z * zaxis.z ;
	r.m_Ori = m_eye;

	return r;
}
