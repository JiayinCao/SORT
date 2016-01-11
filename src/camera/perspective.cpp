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
#include "imagesensor/imagesensor.h"

IMPLEMENT_CREATOR( PerspectiveCamera );

void PerspectiveCamera::_init()
{
	// set camera type
	m_type = CT_PERSPECTIVE;
	
	// setup intial value for data
	m_lensRadius = 0.0f;	// by default , depth of field is not enabled
	m_interaxial = 0.0f;	// by default , stereo vision is not enabled
	m_fov = PI * 0.25f;
	m_redOnLeft = true; // by default , red glass is on the left
	
	_registerAllProperty();
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
			return Spectrum( 1.0f , 0.0f , 0.0f );
		else
			return Spectrum( 0.0f , 1.0f , 1.0f );
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
	Sort_Assert( m_imagesensor != 0 );
	
	// update focal distance every time
	float fd = ( m_target - m_eye ).Length();
	
	// check if focal distance is zero
	Sort_Assert(fd!=0.0f);
	
	float hir = m_interaxial * 0.5f;
	if( pass_id == 0 )
		hir = -hir;
	
	x += ps.img_u;
	y += ps.img_v;
	
	float w = (float)m_imagesensor->GetWidth();
	float h = (float)m_imagesensor->GetHeight();
	float aspect = w/h * m_aspectRatioW/m_aspectRatioH;

	float yScale = 0.5f / tan( m_fov * 0.5f );
	float xScale = yScale / aspect;

	// handle different sensor size
	float imagePlaneDist = 0.0f;
	if( m_sensorW && m_sensorH )
	{
		if( m_aspectFit == 1 )	// horizontal fit
		{
			xScale = 0.5f / tan( m_fov * 0.5f );
			yScale = xScale * aspect;
			imagePlaneDist = xScale * w;
		}else if( m_aspectFit == 2 )	// vertical fit
		{
			yScale = 0.5f / tan( m_fov * 0.5f );
			xScale = yScale / aspect;
			imagePlaneDist = yScale * h;
		}else	// auto fit
		{
			if( aspect < 1 )
			{
				yScale = 0.5f / tan( m_fov * 0.5f );
				xScale = yScale / aspect;
				imagePlaneDist = yScale * h;
			}else if( aspect > 1 )
			{
				xScale = 0.5f / tan( m_fov * 0.5f );
				yScale = xScale * aspect;
				imagePlaneDist = xScale * w;
			}
		}
	}

	Vector v;
	v.x = ( x / w - 0.5f ) / xScale - hir / fd;
	v.y = -1.0f * ( ( y / h - 0.5f ) ) / yScale ;
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

	// calculate the pdf for camera ray
	const float cosAtCamera = Dot( zaxis , r.m_Dir );
	const float imagePointToCameraDist = imagePlaneDist / cosAtCamera;
	const float imageToSolidAngleFactor = imagePointToCameraDist * imagePointToCameraDist / cosAtCamera;

	// the pdf of the ray
	r.m_fPDF = imageToSolidAngleFactor;

	return r;
}

// get eye direction
Vector PerspectiveCamera::GetForward()
{
	return (m_target-m_eye).Normalize();
}

// register all properties
void PerspectiveCamera::_registerAllProperty()
{
	_registerProperty( "aspect" , new AspectProperty( this ) );
	_registerProperty( "sensorsize" , new SensorSizeProperty( this ) );
	_registerProperty( "eye" , new EyeProperty( this ) );
	_registerProperty( "up" , new UpProperty( this ) );
	_registerProperty( "target" , new TargetProperty( this ) );
	_registerProperty( "fov" , new FovProperty( this ) );
	_registerProperty( "len" , new LenProperty( this ) );
	_registerProperty( "interaxial" , new InteraxialProperty( this ) );
}

// get camera coordinate according to a view direction in world space
Vector2i PerspectiveCamera::GetScreenCoord(Vector dir , float* pdf)
{
	// get view axis
	Vector zaxis = Normalize(m_target - m_eye);
	Vector xaxis = Cross(m_up, zaxis).Normalize();
	Vector yaxis = Cross(zaxis, xaxis);

	// get view space dir
	Vector vs_dir;
	vs_dir.x = dir.x * xaxis.x + dir.y * xaxis.y + dir.z * xaxis.z;
	vs_dir.y = dir.x * yaxis.x + dir.y * yaxis.y + dir.z * yaxis.z;
	vs_dir.z = dir.x * zaxis.x + dir.y * zaxis.y + dir.z * zaxis.z;
	vs_dir /= vs_dir.z;

	float w = (float)m_imagesensor->GetWidth();
	float h = (float)m_imagesensor->GetHeight();
	float aspect = w / h * m_aspectRatioW / m_aspectRatioH;

	float yScale = 0.5f / tan(m_fov * 0.5f);
	float xScale = yScale / aspect;

	// handle different sensor size
	float imagePlaneDist = 0.0f;
	if( m_sensorW && m_sensorH )
	{
		if( m_aspectFit == 1 )	// horizontal fit
		{
			xScale = 0.5f / tan( m_fov * 0.5f );
			yScale = xScale * aspect;
			imagePlaneDist = xScale * w;
		}else if( m_aspectFit == 2 )	// vertical fit
		{
			yScale = 0.5f / tan( m_fov * 0.5f );
			xScale = yScale / aspect;
			imagePlaneDist = yScale * h;
		}else	// auto fit
		{
			if( aspect < 1 )
			{
				yScale = 0.5f / tan( m_fov * 0.5f );
				xScale = yScale / aspect;
				imagePlaneDist = yScale * h;
			}else if( aspect > 1 )
			{
				xScale = 0.5f / tan( m_fov * 0.5f );
				yScale = xScale * aspect;
				imagePlaneDist = xScale * w;
			}
		}
	}

	Vector2i coord;
	coord.x = ( vs_dir.x * xScale + 0.5f ) * w;
	coord.y = (-vs_dir.y * yScale + 0.5f ) * h;

	if( pdf )
	{
		// calculate the pdf for camera ray
		const float cosAtCamera = Dot( zaxis , dir );
		const float imagePointToCameraDist = imagePlaneDist / cosAtCamera;
		const float imageToSolidAngleFactor = imagePointToCameraDist * imagePointToCameraDist / cosAtCamera;

		// the pdf of the ray
		*pdf = imageToSolidAngleFactor ;
	}

	return coord;
}