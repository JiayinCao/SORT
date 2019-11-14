/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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
#include "math/bbox.h"

class Primitive;

//! @brief Bounding volume hierarchy node primitives. It is used during BVH construction.
struct Bvh_Primitive {
    Primitive*  primitive;              /**< Primitive lists for this node. */
    Point       m_centroid;             /**< Center point of the BVH node. */

    //! @brief Set primitive.
    //!
    //! @param p    Primitive list holding all primitives in the node.
    void SetPrimitive(Primitive* p){
        primitive = p;
        m_centroid = (p->GetBBox().m_Max + p->GetBBox().m_Min) * 0.5f;
    }

    //! Get bounding box of this primitive set.
    //!
    //! @return     Axis-Aligned bounding box holding all the primitives.
    const BBox& GetBBox() const {
        return primitive->GetBBox();
    }
};

//! @brief Evaluate the SAH value of a specific splitting.
//!
//! @param left         The number of primitives in the left node to be split.
//! @param right        The number of primitives in the right node to be split.
//! @param lbox         Bounding box of the left node to be split.
//! @param rbox         Bounding box of the right node to be split.
//! @param box          Bounding box of the current node.
//! @return             SAH value of the specific split plane.
SORT_FORCEINLINE float sah( unsigned left , unsigned right , const BBox& lbox , const BBox& rbox , const BBox& box ){
    return (left * lbox.HalfSurfaceArea() + right * rbox.HalfSurfaceArea()) / box.HalfSurfaceArea();
}

//! @brief Pick the best split among all possible splits.
//!
//! @param axis         The selected axis id of the picked split plane.
//! @param split_pos    Position of the selected split plane.
//! @param primitives   The buffer hold all primitives.
//! @param node         The node to be split.
//! @param start        The start offset of primitives that the node holds.
//! @param end          The end offset of primitives that the node holds.
//! @return             The SAH value of the selected best split plane.
SORT_FORCEINLINE float pickBestSplit( unsigned& axis , float& splitPos , const Bvh_Primitive* const primitives , const BBox& node_bbox , const unsigned start , const unsigned end ){
    static constexpr unsigned   BVH_SPLIT_COUNT         = 16;
    static constexpr float      BVH_INV_SPLIT_COUNT     = 1.0f / (float)BVH_SPLIT_COUNT;

    BBox inner;
    for(auto i = start ; i < end ; i++ )
        inner.Union( primitives[i].m_centroid );

    auto primitive_num = end - start;
    axis = inner.MaxAxisId();
    auto min_sah = FLT_MAX;

    // distribute the primitives into bins
    unsigned    bin[BVH_SPLIT_COUNT];
    BBox        bbox[BVH_SPLIT_COUNT];
    BBox        rbox[BVH_SPLIT_COUNT-1];
    memset( bin , 0 , sizeof( unsigned ) * BVH_SPLIT_COUNT );
    auto split_start = inner.m_Min[axis];
    auto split_delta = inner.Delta(axis) * BVH_INV_SPLIT_COUNT;
    if( split_delta == 0.0f )
        return FLT_MAX;
    auto inv_split_delta = 1.0f / split_delta;
    for(auto i = start ; i < end ; i++ ){
        auto index = (int)((primitives[i].m_centroid[axis] - split_start) * inv_split_delta);
        index = std::min( index , (int)(BVH_SPLIT_COUNT - 1) );
        ++bin[index];
        bbox[index].Union( primitives[i].GetBBox() );
    }

    rbox[BVH_SPLIT_COUNT-2].Union( bbox[BVH_SPLIT_COUNT-1] );
    for( int i = BVH_SPLIT_COUNT-3; i >= 0 ; i-- )
        rbox[i] = Union( rbox[i+1] , bbox[i+1] );

    auto    left = bin[0];
    auto    lbox = bbox[0];
    auto    pos = split_delta + split_start ;
    for(auto i = 0 ; i < BVH_SPLIT_COUNT - 1 ; i++ ){
        auto sah_value = sah( left , primitive_num - left , lbox , rbox[i] , node_bbox );
        if( sah_value < min_sah ){
            min_sah = sah_value;
            splitPos = pos;
        }
        left += bin[i+1];
        lbox.Union( bbox[i+1] );
        pos += split_delta;
    }

    return min_sah;
}