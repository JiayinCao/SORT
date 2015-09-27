/*
   FileName:      ortho.cpp

   Created Time:  2011-08-23 13:04:21

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "ortho.h"
#include "utility/sassert.h"
#include "sampler/sample.h"
#include "imagesensor/imagesensor.h"

IMPLEMENT_CREATOR( OrthoCamera );

// default constructor
OrthoCamera::OrthoCamera()
{
	_init();
}

// initialize data
void OrthoCamera::_init()
{
	// set type for tha camera
	m_type = CT_ORTHO;
	//set default value
	m_camWidth = 1.0f;
	m_camHeight = 1.0f;
	
	// register properties
	_registerAllProperty();
}

// generate camera ray
Ray OrthoCamera::GenerateRay( unsigned pass_id , float x, float y, const PixelSample &ps) const
{
	x += ps.img_u;
	y += ps.img_v;

	float w = (float)m_imagesensor->GetWidth();
	float h = (float)m_imagesensor->GetHeight();

	x = ( ( x / w ) - 0.5f ) * m_camWidth;
	y = -1.0f * ( ( y / h - 0.5f ) ) * m_camHeight;

	Point ori = world2camera( Point( x , y , 0.0f ) );
	Vector dir = world2camera( Vector( 0.0f , 0.0f , 1.0f ) );

	return Ray( ori , dir );
}

// set the camera range
void OrthoCamera::SetCameraWidth( float w )
{
	Sort_Assert( w > 0.0f );
	m_camWidth = w;
}

void OrthoCamera::SetCameraHeight( float h )
{
	Sort_Assert( h > 0.0f );
	m_camHeight = h;
}

// update transform matrix
void OrthoCamera::_updateTransform()
{
	Vector zaxis = ( m_target - m_eye ).Normalize();
	Vector xaxis = Cross( m_up , zaxis ).Normalize();
	Vector yaxis = Cross( zaxis , xaxis );

	world2camera = Matrix(	xaxis.x , yaxis.x , zaxis.x , m_eye.x ,
							xaxis.y , yaxis.y , zaxis.y , m_eye.y ,
							xaxis.z , yaxis.z , zaxis.z , m_eye.z ,
							0.0f , 0.0f , 0.0f , 1.0f );
}

// register all properties
void OrthoCamera::_registerAllProperty()
{
	_registerProperty( "eye" , new EyeProperty( this ) );
	_registerProperty( "up" , new UpProperty( this ) );
	_registerProperty( "target" , new TargetProperty( this ) );
	_registerProperty( "width" , new WidthProperty( this ) );
	_registerProperty( "height" , new HeightProperty( this ) );
}