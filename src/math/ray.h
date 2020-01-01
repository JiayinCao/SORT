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

#pragma once

#include "core/define.h"
#include "math/vector3.h"
#include "math/point.h"
#include "float.h"
#include "spectrum/spectrum.h"

SORT_STATIC_FORCEINLINE int majorAxis(const Vector3f& v) {
    if (abs(v[0]) > abs(v[1]) && abs(v[0]) > abs(v[2]))
        return 0;
    return abs(v[1]) > abs(v[2]) ? 1 : 2;
}

SORT_STATIC_FORCEINLINE Vector3f permuteAxis(const Vector3f& v, int ax, int ay, int az) {
    return Vector3f(v[ax], v[ay], v[az]);
}

//! @brief  Data structure representing a ray.
class Ray{
public:
    // default constructor
    Ray();
    // constructor from a point and a direction
    // para 'ori'   :   original point of the ray
    // para 'dir'   :   direction of the ray , it's the programmer's responsibility to normalize it
    // para 'depth' :   the depth of the current ray , if not set , default value is 0
    // para 'fmin'  :   the minimum range of the ray . It could be set a very small value to avoid false self intersection
    // para 'fmax'  :   the maximum range of the ray . A ray with 'fmax' not equal to 0 is actually a line segment, usually used for shadow ray.
    Ray( const Point& ori , const Vector& dir , unsigned depth = 0 , float fmin = 0.0f , float fmax = FLT_MAX );
    // copy constructor
    // para 'r' :   a ray to copy
    Ray( const Ray& r );

    // operator to get a point from the ray
    // para 't' :   the distance from the retrieve point if the direction of the ray is normalized.
    // reslut   :   A point ( o + t * d )
    SORT_FORCEINLINE    Point operator ()( float t ) const{
        return m_Ori + t * m_Dir;
    }

    //! @brief  Pre-calculate some cached data for better performance. Only call this function after all ray data is prepared.
    SORT_FORCEINLINE void    Prepare() const{
        m_local_y = majorAxis(m_Dir);
        m_local_z = (m_local_y + 1) % 3;
        m_local_x = (m_local_z + 1) % 3;

        const auto d = permuteAxis(m_Dir, m_local_x, m_local_y, m_local_z);
        m_scale_x = -d.x / d.y;
        m_scale_z = -d.z / d.y;
        m_scale_y = 1.0f / d.y;
    }

// the original point and direction are also public
    // original point of the ray
    Point   m_Ori;
    // direction , the direction of the ray
    Vector  m_Dir;

    // the depth for the ray
    int     m_Depth;

    // the maximum and minimum value in the ray
    float   m_fMin;
    float   m_fMax;

    float   m_fPdfW;
    float   m_fPdfA;
    float   m_fCosAtCamera;

    // importance value of the ray
    Spectrum m_we;

    mutable int     m_local_x , m_local_y , m_local_z;  /**< Id used to identify axis in local coordinate. */
    mutable float   m_scale_x , m_scale_y , m_scale_z;  /**< Scaling along each axis in local coordinate. */
};
