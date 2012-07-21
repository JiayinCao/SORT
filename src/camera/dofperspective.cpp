/*
   FileName:      dofperspective.cpp

   Created Time:  2011-08-10 19:42:21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "dofperspective.h"
#include "texture/rendertarget.h"
#include "utility/sassert.h"
#include "utility/samplemethod.h"
#include "sampler/sample.h"

// generate ray
Ray	DofPerspective::GenerateRay( float x , float y , const PixelSample& ps ) const
{
	// check if there is render target
	Sort_Assert( m_rt != 0 );
	
	x += ps.img_u;
	y += ps.img_v;

	float w = (float)m_rt->GetWidth();
	float h = (float)m_rt->GetHeight();
	float aspect = w / h;

	float yScale = 1.0f / tan( m_fov / 2 );
	float xScale = yScale / aspect;

	Vector v;
	v.x = ( ( x / w ) - 0.5f ) / xScale ;
	v.y = -1.0f * ( ( y / h - 0.5f ) ) / yScale;
	v.z = 1.0f;
	v.Normalize();

	float dis = focalDistance / v.z;

	Vector zaxis = ( m_target - m_eye ).Normalize();
	Vector xaxis = Cross( m_up , zaxis ).Normalize();
	Vector yaxis = Cross( zaxis , xaxis );

	Ray r;
	r.m_Dir.x = v.x * xaxis.x + v.y * yaxis.x + v.z * zaxis.x ;
	r.m_Dir.y = v.x * xaxis.y + v.y * yaxis.y + v.z * zaxis.y ;
	r.m_Dir.z = v.x * xaxis.z + v.y * yaxis.z + v.z * zaxis.z ;
	r.m_Ori = m_eye;
	
	Point target = r(dis);

	float s , t;
	UniformSampleDisk( ps.dof_u , ps.dof_v , s , t );
	s *= lensRadius;
	t *= lensRadius;

	r.m_Ori.x = m_eye.x + s * xaxis.x + t * yaxis.x;
	r.m_Ori.y = m_eye.y + s * xaxis.y + t * yaxis.y;
	r.m_Ori.z = m_eye.z + s * xaxis.z + t * yaxis.z;
	r.m_Dir = Normalize( target - r.m_Ori );

	return r;
}
