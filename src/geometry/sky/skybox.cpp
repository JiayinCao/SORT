/*
   FileName:      skybox.cpp

   Created Time:  2011-08-04 12:51:45

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "skybox.h"
#include "geometry/ray.h"
#include <math.h>

// default constructor
SkyBox::SkyBox()
{
	_registerAllProperty();

	m_up.SetTexCoordFilter( TCF_CLAMP );
	m_down.SetTexCoordFilter( TCF_CLAMP );
	m_left.SetTexCoordFilter( TCF_CLAMP );
	m_right.SetTexCoordFilter( TCF_CLAMP );
	m_front.SetTexCoordFilter( TCF_CLAMP );
	m_back.SetTexCoordFilter( TCF_CLAMP );
}

// evaluate value from sky
Spectrum SkyBox::Evaluate( const Ray& r ) const
{
	float abs_x = fabs( r.m_Dir.x );
	float abs_y = fabs( r.m_Dir.y );
	float abs_z = fabs( r.m_Dir.z );

	if( abs_x > abs_y && abs_x > abs_z )
	{
		if( r.m_Dir.x > 0.0f )
		{
			float u = ( -r.m_Dir.z / r.m_Dir.x + 1.0f ) * 0.5f;
			float v = ( -r.m_Dir.y / r.m_Dir.x + 1.0f ) * 0.5f;
			return m_front.GetColor( u , v );
		}
		else
		{
			float u = ( -r.m_Dir.z / r.m_Dir.x + 1.0f ) * 0.5f;
			float v = ( r.m_Dir.y / r.m_Dir.x + 1.0f ) * 0.5f;
			return m_back.GetColor( u , v );
		}
	}else if( abs_y > abs_x && abs_y > abs_z )
	{
		if( r.m_Dir.y > 0.0f )
		{
			float u = ( r.m_Dir.x / r.m_Dir.y + 1.0f ) * 0.5f;
			float v = ( r.m_Dir.z / r.m_Dir.y + 1.0f ) * 0.5f;
			return m_up.GetColor( u , v );
		}
		else
		{
			float u = ( -r.m_Dir.x / r.m_Dir.y + 1.0f ) * 0.5f;
			float v = ( r.m_Dir.z / r.m_Dir.y + 1.0f ) * 0.5f;
			return m_down.GetColor( u , v );
		}
	}else
	{
		if( r.m_Dir.z > 0.0f )
		{
			float u = ( r.m_Dir.x / r.m_Dir.z + 1.0f ) * 0.5f;
			float v = ( -r.m_Dir.y / r.m_Dir.z + 1.0f ) * 0.5f;
			return m_left.GetColor( u , v );
		}
		else
		{
			float u = ( r.m_Dir.x / r.m_Dir.z + 1.0f ) * 0.5f;
			float v = ( r.m_Dir.y / r.m_Dir.z + 1.0f ) * 0.5f;
			return m_right.GetColor( u , v );
		}
	}

	return Spectrum();
}

// register property
void SkyBox::_registerAllProperty()
{
	_registerProperty( "up" , new UpProperty(this) );
	_registerProperty( "down" , new DownProperty(this) );
	_registerProperty( "left" , new LeftProperty(this) );
	_registerProperty( "right" , new RightProperty(this) );
	_registerProperty( "front" , new FrontProperty(this) );
	_registerProperty( "back" , new BackProperty(this) );
}