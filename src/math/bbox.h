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
#include "math/point.h"
#include "ray.h"

#ifdef SSE_ENABLED
    #include <nmmintrin.h>
#endif

//! @brief  Axis Aligned Bounding Box.
class BBox{
public:
    //! @brief  Default constructor
    //!
    //! Default constructor will set the data in a way that it is invalid.
    BBox();

    //! @brief  Constructor from two points
    //!
    //! @param p0       One of the eight corners of the AABB.
    //! @param p1       The other of the eight corners of the AABB.
    //! @param sorted   If it is true, p0 is min and p1 is max.
    BBox( const Point& p0 , const Point& p1 , bool sorted = true );
    
    //! @brief  Copy constructor.
    //!
    //! @param bbox     Another instance of bounding box to copy from.
    BBox( const BBox& bbox );

    //! @brief  Whether a point is inside the AABB.
    //!
    //! @param  p       The point to be tested.
    //! @param  delta   A small value to avoid float precision problem.
    //! @return         It returns true if the point is within the AABB.
    bool        IsInBBox( const Point& p , float delta ) const;

    // get the surface area of the bounding box
    // result : the surface area of the bounding box

    //! @brief  The surface area of the bounding box.
    //!
    //! @return         The surface area of the bounding box.
    float       SurfaceArea() const;
    
    //! @brief  Half of the surface area of the bounding box.
    //!
    //! @return         Half of the surface area of the bounding box.
    float       HalfSurfaceArea() const;

    //! @brief  The volume of this bounding box.
    //!
    //! @return         The volume of the bounding box.
    float       Volumn() const;

    //! @brief  The axis id with maximum edge.
    //!
    //! @return         The axis id ( 0 -> x , 1 -> y , 2 -> z ) that has the longest edge.
    unsigned    MaxAxisId() const;

    //! @brief  Enlarge the bounding box to contain the point.
    //!
    //! @param  p       The point to contain.
    void        Union( const Point& p );

    //! @brief  Enlarge the bounding box to contain the other bounding box.
    //!
    //! @param box      The other bounding box to contain.
    void        Union( const BBox& box );

    //! @brief  Length of the edge along a specific axis.
    //!
    //! @return         The length of the edge along specific edge.
    float       Delta( unsigned k ) const;

    //! @brief  Reset the bounding box so that it is an invalid one.
    void        InvalidBBox();

    //! @param  delta   The half delta to expend along each direction.
    //! @brief  Expend the bounding box.
    //!
    //! @param delta    Half of the length to expend along every axis.
    void        Expend( float delta );

public:
    // the minium and maxium point of the bounding box
    Point   m_Min;
    Point   m_Max;
};

SORT_FORCEINLINE BBox Union( const BBox& bbox , const Point& p ){
    BBox box;
    for( unsigned i = 0 ; i < 3 ; i++ ){
        if( p[i] < bbox.m_Min[i] )
            box.m_Min[i] = p[i];
        if( p[i] > bbox.m_Max[i] )
            box.m_Max[i] = p[i];
    }
    return box;
}

SORT_FORCEINLINE BBox Union( const BBox& bbox0 , const BBox& bbox1 ){
    BBox result;
    for( int i = 0 ; i < 3 ; i++ ){
        result.m_Min[i] = std::min( bbox0.m_Min[i] , bbox1.m_Min[i] );
        result.m_Max[i] = std::max( bbox0.m_Max[i] , bbox1.m_Max[i] );
    }
    return result;
}

SORT_FORCEINLINE float Intersect( const Ray& ray , const BBox& bb , float* fmax = nullptr ){
    //set default value for tmax and tmin
    float tmax = ray.m_fMax;
    float tmin = ray.m_fMin;

    for( unsigned axis = 0 ; axis < 3 ; axis ++ ){
        if( ray.m_Dir[axis] < 0.00001f && ray.m_Dir[axis] > -0.00001f ){
            if( ray.m_Ori[axis] > bb.m_Max[axis] || ray.m_Ori[axis] < bb.m_Min[axis] )
                return -1.0f;
        }else{
            float ood = 1.0f / ray.m_Dir[axis];
            float t1 = ( bb.m_Max[axis] - ray.m_Ori[axis] ) * ood;
            float t2 = ( bb.m_Min[axis] - ray.m_Ori[axis] ) * ood;

            if( t1 > t2 ){
                float t = t1;
                t1 = t2;
                t2 = t;
            }

            tmin = std::max( t1 , tmin );
            tmax = std::min( t2 , tmax );

            if( tmin > tmax )
                return -1.0f;
        }
    }

    if( fmax )
        *fmax = tmax;

    return tmin;
}