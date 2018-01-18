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

#include "ortho.h"
#include "utility/sassert.h"
#include "sampler/sample.h"
#include "imagesensor/imagesensor.h"

IMPLEMENT_CREATOR( OrthoCamera );

// default constructor
OrthoCamera::OrthoCamera()
{
    // set type for tha camera
    m_type = CT_ORTHO;
    
    // register properties
    registerAllProperty();
}

// generate camera ray
Ray OrthoCamera::GenerateRay( float x, float y, const PixelSample &ps) const
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
	sAssert( w > 0.0f , CAMERA );
	m_camWidth = w;
}

void OrthoCamera::SetCameraHeight( float h )
{
	sAssert( h > 0.0f , CAMERA );
	m_camHeight = h;
}

// update transform matrix
void OrthoCamera::updateTransform()
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
void OrthoCamera::registerAllProperty()
{
	_registerProperty( "eye" , new EyeProperty( this ) );
	_registerProperty( "up" , new UpProperty( this ) );
	_registerProperty( "target" , new TargetProperty( this ) );
	_registerProperty( "width" , new WidthProperty( this ) );
	_registerProperty( "height" , new HeightProperty( this ) );
}
