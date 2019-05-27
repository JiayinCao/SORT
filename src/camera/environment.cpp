/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <math.h>
#include "environment.h"
#include "math/vector3.h"
#include "math/ray.h"
#include "core/sassert.h"
#include "core/define.h"
#include "sampler/sample.h"
#include "core/globalconfig.h"

// generate a ray given a pixel
Ray EnvironmentCamera::GenerateRay( float x , float y , const PixelSample& ps ) const{
    x += ps.img_u;
    y += ps.img_v;

    // generate ray
    float theta = PI * y / (float)g_resultResollutionHeight;
    float phi = 2 * PI * x / (float)g_resultResollutionWidth;
    Vector dir( sinf( theta ) * cosf( phi ) , cosf( theta ) , sinf( theta ) * sinf( phi ) );
    Ray r( m_eye , dir );

    // transform the ray
    r = m_transform(r);

    return r;
}