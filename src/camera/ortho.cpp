/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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
#include "core/sassert.h"
#include "core/globalconfig.h"
#include "sampler/sample.h"

// generate camera ray
Ray OrthoCamera::GenerateRay( float x, float y, const PixelSample &ps) const{
    x += ps.img_u;
    y += ps.img_v;

    const auto w = (float)m_image_width;
    const auto h = (float)m_image_height;

    x = ( ( x / w ) - 0.5f ) * m_camWidth;
    y = -1.0f * ( ( y / h - 0.5f ) ) * m_camHeight;

    const auto ori = world2camera.TransformPoint( Point( x , y , 0.0f ) );
    const auto dir = world2camera.TransformVector( Vector( 0.0f , 0.0f , 1.0f ) );

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
    Vector xaxis = cross( m_up , zaxis ).Normalize();
    Vector yaxis = cross( zaxis , xaxis );

    world2camera = Matrix(  xaxis.x , yaxis.x , zaxis.x , m_eye.x ,
                            xaxis.y , yaxis.y , zaxis.y , m_eye.y ,
                            xaxis.z , yaxis.z , zaxis.z , m_eye.z ,
                            0.0f , 0.0f , 0.0f , 1.0f );
}