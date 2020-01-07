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

#include "accelerator.h"
#include "bvh_utils.h"
#include "core/primitive.h"

#if defined(SIMD_SSE_IMPLEMENTATION) && defined(SIMD_AVX_IMPLEMENTATION)
static_assert(false, "More than one SIMD version is defined before including fast_bvh.h");
#endif

#if defined(QBVH_IMPLEMENTATION) || defined(OBVH_IMPLEMENTATION)

#if defined(QBVH_IMPLEMENTATION)
#define Fast_Bvh_Node   Qbvh_Node
#define FBVH_CHILD_CNT  4
#endif

#if defined(OBVH_IMPLEMENTATION)
#define Fast_Bvh_Node   Obvh_Node
#define FBVH_CHILD_CNT  8
#endif

#ifdef SIMD_BVH_IMPLEMENTATION
struct Fast_Bvh_Node_Deallocator{
    void operator()(void* p){
        free_aligned(p);
    }
};

struct Fast_Bvh_Node;
using Fast_Bvh_Node_Ptr = std::unique_ptr<Fast_Bvh_Node,Fast_Bvh_Node_Deallocator>;

#else
struct Fast_Bvh_Node;
using Fast_Bvh_Node_Ptr = std::unique_ptr<Fast_Bvh_Node>;
#endif

struct Fast_Bvh_Node {
#ifdef SIMD_BVH_IMPLEMENTATION
    using Simd_Triangle_Container   = std::unique_ptr<Simd_Triangle[]>;
    using Simd_Line_Container       = std::unique_ptr<Simd_Line[]>;
    Simd_BBox                       bbox;                       /**< Bounding boxes of its four children. */
    Simd_Triangle_Container         tri_list;
    Simd_Line_Container             line_list;
    unsigned int                    tri_cnt = 0;
    unsigned int                    line_cnt = 0;
    std::vector<const Primitive*>   other_list;
#else
    BBox                            bbox[FBVH_CHILD_CNT];       /**< Bounding boxes of its children. */
#endif

    Fast_Bvh_Node_Ptr               children[FBVH_CHILD_CNT];   /**< Children of its four nodes. */

    unsigned                        pri_cnt = 0;                /**< Number of primitives in the node. */
    unsigned                        pri_offset = 0;             /**< Offset of primitives in the buffer. */
    unsigned                        child_cnt = 0;              /**< 0 means it is a leaf node. */

    //! @brief  Constructor.
    //!
    //! @param  offset      The offset of the first primitive in the whole buffer.
    //! @param  cnt         Number of primitives in the node.
    Fast_Bvh_Node(unsigned offset, unsigned cnt) : pri_cnt(cnt), pri_offset(offset) {}

    //! @brief  Default constructor.
    Fast_Bvh_Node() : pri_cnt(0), pri_offset(0), child_cnt(0) {}  
};

#ifdef SIMD_BVH_IMPLEMENTATION
    static_assert( sizeof( Fast_Bvh_Node ) % SIMD_ALIGNMENT == 0 , "Incorrect size of Fast_Bvh_Node." );
#endif

#endif

//! @brief Fast Bounding volume hierarchy.
/**
 * Shallow Bounding Volume Hierarchies for Fast SIMD Ray Tracing of Incoherent Rays
 * https://www.uni-ulm.de/fileadmin/website_uni_ulm/iui.inst.100/institut/Papers/QBVH.pdf
 * 
 * Being different from traditional BVH, a quad/oct BVH node has four/eight children instead of two, making it a 4/8-ary tree, instead of 
 * a binary tree. It easily opens the door for SSE/AVX optimization during BVH traversal since we can do ray-AABB intersection 
 * four/eight times more efficient. And also we can do the same to primitive ray intersection, instead of doing it one at a time,
 * QBVH/OBVH will check four/eight primitives at a time, boosting the performance of ray intersection test.
 */
class Fbvh : public Accelerator{
public:
#ifdef QBVH_IMPLEMENTATION
    DEFINE_RTTI( Qbvh , Accelerator );
#endif
#ifdef OBVH_IMPLEMENTATION
    DEFINE_RTTI( Obvh , Accelerator );
#endif

    //! @brief Get intersection between the ray and the primitive set using QBVH/OBVH.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an existed intersection, if intersect is not nullptr, it will fill the
    //! structure and return the nearest intersection. If intersect is nullptr, it will stop
    //! as long as one intersection is found, it is not necessary to be the nearest one.
    //! False will be returned if there is no intersection at all.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, it
    //!                     stops as long as it finds an intersection. It is faster than
    //!                     the one with intersection information data and suitable for
    //!                     shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise
    //!                     it returns false.
    bool    GetIntersect( const Ray& r , Intersection* intersect ) const override;

#ifndef ENABLE_TRANSPARENT_SHADOW
    //! @brief This is a dedicated interface for detecting shadow rays.
    //!
    //! Instead of merging the interface with 'GetIntersect', this is a separate interface purely for occlusion detection.
    //! There is a need for it so that we can achieve better performance. There will be less branch in this interfaces and
    //! most importantly the traversed node doesn't need to be sorted.
    //!
    //! @param r            The ray to be tested.
    //! @return             Whether the ray is occluded by anything.
    bool    IsOccluded(const Ray& r) const override;
#endif

    //! @brief Get multiple intersections between the ray and the primitive set using spatial data structure.
    //!
    //! This is a specific interface designed for SSS during disk ray casting. Without this interface, the algorithm has to use the
    //! above one to acquire all intersections in a brute force way, which obviously introduces quite some duplicated work.
    //! The intersection returned doesn't guarantee the order of the intersection of the results, but it does guarantee to get the
    //! nearest N intersections.
    //!
    //! @param  r           The input ray to be tested.
    //! @param  intersect   The intersection result that holds all intersection.
    //! @param  matID       We are only interested in intersection with the same material, whose material id should be set to matID.
    void    GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID = INVALID_SID ) const override;

    //! @brief Build BVH structure in O(N*lg(N)).
    //!
    //! @param scene    The rendering scene.
    void    Build(const Scene& scene) override;

    //! @brief      Serializing data from stream.
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation,
    //!             it could come from different places.
    void    Serialize( IStreamBase& stream ) override{
        stream >> m_maxNodeDepth;
        stream >> m_maxPriInLeaf;
    }

private:
    /**< Primitive list during QBVH/OBVH construction. */
    std::unique_ptr<Bvh_Primitive[]>    m_bvhpri = nullptr;

    /**< Root node of the BVH. */
    Fast_Bvh_Node_Ptr                   m_root;

    /**< Maximum primitives in a leaf node. During BVH construction, a node with less primitives will be marked as a leaf node. */
    unsigned                            m_maxPriInLeaf = 8;
    /**< Maximum depth of node in BVH. */
    unsigned                            m_maxNodeDepth = 16;

    /**< Depth of the QBVH/OBVH. */
    unsigned                            m_depth = 0;

    //! @brief Split current QBVH/OBVH node.
    //!
    //! @param node         The QBVH/OBVH node to be split.
    //! @param node_bbox    The bounding box of the node.
    //! @param depth        The current depth of the node. Starting from 1 for root node.
    void    splitNode( Fbvh_Node* const node , const BBox& node_bbox , unsigned depth );

    //! @brief Mark the current node as leaf node.
    //!
    //! @param node         The BVH node to be marked as leaf node.
    //! @param start        The start offset of primitives that the node holds.
    //! @param end          The end offset of primitives that the node holds.
    //! @param depth        Depth of the current node.
    void    makeLeaf( Fbvh_Node* const node , unsigned start , unsigned end , unsigned depth );

#ifdef SIMD_BVH_IMPLEMENTATION
    //! @brief A helper function calculating bounding box of a node.
    //!
    //! @param children     The children nodes
    //! @return             The 4/8 bounding box of the node, there could be degenerated ones if there is no four children.
    Simd_BBox   calcBoundingBoxSIMD(const Fast_Bvh_Node_Ptr* children) const;
#endif

#ifdef QBVH_IMPLEMENTATION
    SORT_STATS_ENABLE( "Spatial-Structure(QBVH)" )
#endif
#ifdef OBVH_IMPLEMENTATION
    SORT_STATS_ENABLE( "Spatial-Structure(OBVH)" )
#endif
};
