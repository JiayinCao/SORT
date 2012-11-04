/*
   FileName:      perspective.cpp

   Created Time:  2011-08-04 12:52:08

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "perspective.h"
#include "texture/rendertarget.h"
#include <math.h>
#include "utility/sassert.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

void PerspectiveCamera::_init()
{
	// set camera type
	m_type = CAMERA_PERSPECTIVE;
	
	// setup intial value for data
	m_lensRadius = 0.0f;	// by default , depth of field is not enabled
	m_interaxial = 0.0f;	// by default , stereo vision is not enabled
	m_fov = PI * 0.25f;
	m_redOnLeft = true; // by default , red glass is on the left
}

// get pass number.
unsigned PerspectiveCamera::GetPassCount() const
{
	return (m_interaxial==0.0f)?1:2;
}

// get pass filter
Spectrum PerspectiveCamera::GetPassFilter( unsigned id ) const
{
	if( GetPassCount() == 2 )
	{
		if( ( m_redOnLeft && id == 0 ) ||
		    ( m_redOnLeft == false && id == 1 ) )
			return Spectrum( 1.0f , 0.5f , 0.0f );
		else
			return Spectrum( 0.0f , 0.5f , 1.0f );
	}
	
	return 1.0f;
}

// by default, red glass is one the left.
void PerspectiveCamera::SwitchGlass( bool redOnLeft )
{
	m_redOnLeft = redOnLeft;
}

// generate ray
Ray	PerspectiveCamera::GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const
{
	// check if there is render target
	Sort_Assert( m_rt != 0 );
	
	// update focal distance every time
	float fd = ( m_target - m_eye ).Length();
	
	// check if focal distance is zero
	Sort_Assert(fd);
	
	float hir = m_interaxial * 0.5f;
	if( pass_id == 0 )
		hir = -hir;
	
	x += ps.img_u;
	y += ps.img_v;
	
	float w = (float)m_rt->GetWidth();
	float h = (float)m_rt->GetHeight();
	float aspect = w / h;
	
	float yScale = 1.0f / tan( m_fov / 2 );
	float xScale = yScale / aspect;
	
	Vector v;
	v.x = ( ( x / w ) - 0.5f ) / xScale - hir / fd;
	v.y = -1.0f * ( ( y / h - 0.5f ) ) / yScale;
	v.z = 1.0f;
	v.Normalize();
	
	Vector zaxis = ( m_target - m_eye ) / fd;
	Vector xaxis = Cross( m_up , zaxis ).Normalize();
	Vector yaxis = Cross( zaxis , xaxis );
	
	Ray r;
	r.m_Dir.x = v.x * xaxis.x + v.y * yaxis.x + v.z * zaxis.x ;
	r.m_Dir.y = v.x * xaxis.y + v.y * yaxis.y + v.z * zaxis.y ;
	r.m_Dir.z = v.x * xaxis.z + v.y * yaxis.z + v.z * zaxis.z ;
	r.m_Ori = m_eye + hir * xaxis;

	// NOTE : the dof combines with stereo vision is not physically correct, but it looks cool.
	// caculate dof if radius of lens is not zero
	if( m_lensRadius != 0 )
	{
		Point target = r(fd / v.z);
	
		float s , t;
		UniformSampleDisk( ps.dof_u , ps.dof_v , s , t );
		s *= m_lensRadius;
		t *= m_lensRadius;
	
		r.m_Ori.x = m_eye.x + s * xaxis.x + t * yaxis.x;
		r.m_Ori.y = m_eye.y + s * xaxis.y + t * yaxis.y;
		r.m_Ori.z = m_eye.z + s * xaxis.z + t * yaxis.z;
		r.m_Ori += hir * xaxis;
		r.m_Dir = Normalize( target - r.m_Ori );
	}

	return r;
}
