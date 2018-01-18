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
#include "environment.h"
#include "math/vector3.h"
#include "geometry/ray.h"
#include "utility/sassert.h"
#include "utility/define.h"
#include "imagesensor/imagesensor.h"
#include <math.h>
#include "sampler/sample.h"

IMPLEMENT_CREATOR( EnvironmentCamera );

// generate a ray given a pixel
Ray	EnvironmentCamera::GenerateRay( float x , float y , const PixelSample& ps ) const
{
	sAssert( m_imagesensor != 0 , CAMERA );

	x += ps.img_u;
	y += ps.img_v;

	// generate ray
	float theta = PI * y / m_imagesensor->GetHeight();
	float phi = 2 * PI * x / m_imagesensor->GetWidth();
	Vector dir( sinf( theta ) * cosf( phi ) , cosf( theta ) , sinf( theta ) * sinf( phi ) );
	Ray r( m_eye , dir );

	// transform the ray
	r = m_transform(r);

	return r;
}

// register all properties
void EnvironmentCamera::registerAllProperty()
{
	_registerProperty( "eye" , new EyeProperty( this ) );
}
