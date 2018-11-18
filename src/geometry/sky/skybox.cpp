/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header
#include "skybox.h"
#include "math/vector3.h"
#include <math.h>
#include "utility/samplemethod.h"

IMPLEMENT_CREATOR( SkyBox );

// initialize default value
void SkyBox::_init()
{
	_registerAllProperty();
	m_up.SetTexCoordFilter( TCF_CLAMP );
	m_down.SetTexCoordFilter( TCF_CLAMP );
	m_left.SetTexCoordFilter( TCF_CLAMP );
	m_right.SetTexCoordFilter( TCF_CLAMP );
	m_front.SetTexCoordFilter( TCF_CLAMP );
	m_back.SetTexCoordFilter( TCF_CLAMP );
}

// release
void SkyBox::_release()
{
}

// evaluate value from sky
Spectrum SkyBox::Evaluate( const Vector& vec ) const
{
	float abs_x = fabs( vec.x );
	float abs_y = fabs( vec.y );
	float abs_z = fabs( vec.z );

	if( abs_x > abs_y && abs_x > abs_z )
	{
		if( vec.x > 0.0f )
		{
			float u = ( -vec.z / vec.x + 1.0f ) * 0.5f;
			float v = ( -vec.y / vec.x + 1.0f ) * 0.5f;
			return m_front.GetColorFromUV( u , v );
		}
		else
		{
			float u = ( -vec.z / vec.x + 1.0f ) * 0.5f;
			float v = ( vec.y / vec.x + 1.0f ) * 0.5f;
			return m_back.GetColorFromUV( u , v );
		}
	}else if( abs_y > abs_x && abs_y > abs_z )
	{
		if( vec.y > 0.0f )
		{
			float u = ( vec.x / vec.y + 1.0f ) * 0.5f;
			float v = ( vec.z / vec.y + 1.0f ) * 0.5f;
			return m_up.GetColorFromUV( u , v );
		}
		else
		{
			float u = ( -vec.x / vec.y + 1.0f ) * 0.5f;
			float v = ( vec.z / vec.y + 1.0f ) * 0.5f;
			return m_down.GetColorFromUV( u , v );
		}
	}else
	{
		if( vec.z > 0.0f )
		{
			float u = ( vec.x / vec.z + 1.0f ) * 0.5f;
			float v = ( -vec.y / vec.z + 1.0f ) * 0.5f;
			return m_left.GetColorFromUV( u , v );
		}
		else
		{
			float u = ( vec.x / vec.z + 1.0f ) * 0.5f;
			float v = ( vec.y / vec.z + 1.0f ) * 0.5f;
			return m_right.GetColorFromUV( u , v );
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

// get the average radiance
Spectrum SkyBox::GetAverage() const
{
	// note : 	there is only a rough simulation of the average radiance.
	//			because a skylight use a rough simulation for power, a more
	//			elegent way of computing the average doesn't make much sense.
	return 0.16666666f * ( 	m_up.GetAverage() + m_down.GetAverage() +
							m_front.GetAverage() + m_back.GetAverage() +
							m_left.GetAverage() + m_right.GetAverage() );
}

// sample direction
Vector SkyBox::sample_v( float u , float v , float* pdf , float* area_pdf ) const
{
	// note :	Actually , it's very ugly to sample a ray on the sphere uniformly
	//			Noises could be reduced by sampling according to the radiance from image.
	Vector wi = UniformSampleSphere( u , v );
	if( pdf ) *pdf = UniformSpherePdf();
	if( area_pdf ) *area_pdf = 0.25f * INV_PI;

	return wi;
}

// get the pdf
float SkyBox::Pdf( const Vector& wi ) const
{
	return UniformSpherePdf();
}
