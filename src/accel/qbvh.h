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

#include "accelerator.h"
#include "core/primitive.h"
#include "bvh_utils.h"

#define QBVH_CHILD_CNT		4

static_assert( QBVH_CHILD_CNT > 1 , "Qbvh node has to have at least two children." );

struct Qbvh_Node {
#if SSE_ENABLED
	BBox4						bbox;						/**< Bounding boxes of its four children. */
#else
	BBox                        bbox[QBVH_CHILD_CNT];       /**< Bounding boxes of its children. */
#endif
	std::unique_ptr<Qbvh_Node>  children[QBVH_CHILD_CNT];   /**< Children of its four nodes. */

	unsigned                    pri_cnt = 0;				/**< Number of primitives in the node. */
	unsigned                    pri_offset = 0;				/**< Offset of primitives in the buffer. */
	bool                        leaf_node = false;			/**< Whether this is a leaf node. */

    //! @brief  Constructor.
    //!
    //! @param  offset      The offset of the first primitive in the whole buffer.
    //! @param  cnt         Number of primitives in the node.
    Qbvh_Node( unsigned offset , unsigned cnt ) : pri_cnt(cnt),pri_offset(offset){
        for( int i = 0 ; i < QBVH_CHILD_CNT ; ++i )
            children[i] = nullptr;
    }
};


//! @brief Quad Bounding volume hierarchy.
/**
 * Shallow Bounding Volume Hierarchies for Fast SIMD Ray Tracing of Incoherent Rays
 * https://www.uni-ulm.de/fileadmin/website_uni_ulm/iui.inst.100/institut/Papers/QBVH.pdf
 * 
 * Being different from traditional BVH, a quad BVH node has four children instead of two, making it a 4-ary tree, instead of 
 * a binary tree. It easily opens the door for SSE optimization during BVH traversal since we can do ray-AABB intersection 
 * four times more efficient. And also we can do the same to primitive ray intersection, instead of doing it one at a time,
 * QBVH will check four/eight primitives at a time, boosting the performance of ray intersection test.
 */
class Qbvh : public Accelerator{
public:
    DEFINE_RTTI( Qbvh , Accelerator );

    //! @brief Get intersection between the ray and the primitive set using QBVH.
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
    void	GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID = INVALID_SID ) const override;

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
    /**< Primitive list during QBVH construction. */
    std::unique_ptr<Bvh_Primitive[]>    m_bvhpri = nullptr;

    /**< Root node of the BVH. */
    std::unique_ptr<Qbvh_Node>          m_root;

    /**< Maximum primitives in a leaf node. During BVH construction, a node with less primitives will be marked as a leaf node. */
    unsigned                            m_maxPriInLeaf = 8;
    /**< Maximum depth of node in BVH. */
    unsigned                            m_maxNodeDepth = 16;

    //! @brief Split current QBVH node.
    //!
    //! @param node         The QBVH node to be split.
    //! @param node_bbox    The bounding box of the node.
    //! @param depth        The current depth of the node. Starting from 1 for root node.
    void    splitNode( Qbvh_Node* const node , const BBox& node_bbox , unsigned depth );

    //! @brief Mark the current node as leaf node.
    //!
    //! @param node         The BVH node to be marked as leaf node.
    //! @param start        The start offset of primitives that the node holds.
    //! @param end          The end offset of primitives that the node holds.
    void    makeLeaf( Qbvh_Node* const node , unsigned start , unsigned end );

#ifdef SSE_ENABLED
	//! @brief A helper function calculating bounding box of a node.
	//!
	//! @param node0        First child of the current node.
	//! @param node1		Second child of the current node.
	//! @param node2		Third child of the current node.
	//! @param node3		Forth child of the current node.
	//! @return             The 4 bounding box of the node, there could be degenerated ones if there is no four children.
	BBox4	calcBoundingBox4(const Qbvh_Node* const node0, const Qbvh_Node* const node1, const Qbvh_Node* const node2, const Qbvh_Node* const node3) const;
#endif

    //! @brief A recursive function that traverses the BVH node.
    //!
    //! @param node         The root node of the (sub)tree to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The structure holding the intersection information. If empty pointer is passed,
    //!                     it will return as long as one intersection is found and it won't be necessary to be
    //!                     the nearest one.
    //! @param fmin         The minimum range along the ray.
    //! @return             True if there is intersection, otherwise it will return false.
    bool    traverseNode( const Qbvh_Node* node , const Ray& ray , Intersection* intersect , float fmin ) const;

    //! @brief A recursive helper function that traverse the BVH to find all intersections.
    //!
    //! @param node         The root node of the (sub)tree to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The result intersections.
    //! @param fmin         The minimum range along the ray, any intersection before it will be ignored.
    //! @param              Material ID to avoid if it is not invalid.
    void    traverseNode( const Qbvh_Node* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , const StringID matID ) const;
    
    SORT_STATS_ENABLE( "Spatial-Structure(QBVH)" )
};
