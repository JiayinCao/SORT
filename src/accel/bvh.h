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
#include "core/primitive.h"
#include "bvh_utils.h"

//! @brief Bounding volume hierarchy.
/**
 * A bounding volume hierarchy (BVH) is a tree structure on a set of geometric objects.
 * All geometric objects are wrapped in bounding volumes that form the leaf nodes of the
 * tree. These nodes are then grouped as small sets and enclosed within larger bounding
 * volumes. These, in turn, are also grouped and enclosed within other larger bounding
 * volumes in a recursive fashion, eventually resulting in a tree structure with a single
 * bounding volume at the top of the tree. Bounding volume hierarchies are used to support
 * several operations on sets of geometric objects efficiently, such as in collision
 * detection or ray tracing.
 * BVH(Bounding volume hierarchy) is a classic spatial acceleration structure commonly
 * used in ray tracing applications. This is a sah BVH implementation whose construction
 * is in O(N*lg(N)).
 * Please refer to this paper
 * <a href="http://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf">
 * On fast Construction of SAH-based Bounding Volume Hierarchies</a> for further details.
 */
class Bvh : public Accelerator{
    //! @brief Bounding volume hierarchy node.
    struct Bvh_Node {
        BBox                        bbox;                   /**< Bounding box of the BVH node. */
        unsigned                    pri_num = 0;            /**< Number of primitives in the BVH node. */
        unsigned                    pri_offset = 0;         /**< Offset in the primitive buffer. It is 0 for interior nodes. */
        std::unique_ptr<Bvh_Node>   left = nullptr;         /**< Left child of the BVH node. */
        std::unique_ptr<Bvh_Node>   right = nullptr;        /**< Right child of the BVH node. */
    };

public:
    DEFINE_RTTI( Bvh , Accelerator );

    //! @brief Get intersection between the ray and the primitive set using BVH.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an existed intersection, if intersect is not empty, it will fill the
    //! structure and return the nearest intersection.If intersect is empty, it will stop
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
    bool    GetIntersect( const Ray& r , SurfaceInteraction& intersect ) const override;

#ifndef ENABLE_TRANSPARENT_SHADOW
    //! @brief This is a dedicated interface for detecting shadow rays.
    //!
    //! Instead of merging the interface with 'GetIntersect', this is a separate interface purely for occlusion detection.
    //! There is a need for it so that we can achieve better performance. There will be less branch in this interfaces and
    //! most importantly the traversed node doesn't need to be sorted.
    //!
    //! @param r            The ray to be tested.
    //! @return             Whether the ray is occluded by anything.
    bool IsOccluded( const Ray& r ) const override;
#endif

    //! @brief Get multiple intersections between the ray and the primitive set using spatial data structure.
    //!
    //! This is a specific interface designed for SSS during disk ray casting. Without this interface, the algorithm has to use the
    //! above one to acquire all intersections in a brute force way, which obviously introduces quite some duplicated work.
    //! The intersection returned doesn't guarantee the order of the intersection of the results, but it does guarantee to get the
    //! nearest N intersections.
    //! WARNING, it is quite possible to find an intersection that is fully transparent and still taking consideration of the light
    //! coming from that point. This is not strictly correct, but I would choose to live with it because it is not worth the extra
    //! performance overhead to fix the problem since it is very minor.
    //!
    //! @param  r           The input ray to be tested.
    //! @param  intersect   The intersection result that holds all intersection.
    //! @param  matID       We are only interested in intersection with the same material, whose material id should be set to matID.
    void    GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID = INVALID_SID ) const override;

    //! @brief Build BVH structure in O(N*lg(N)).
    //!
    //! The BVH construction algorithm is in O(N*lg(N)). Please refer to this paper
    //! <a href = "http://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf">
    //! On fast Construction of SAH - based Bounding Volume Hierarchies< / a> for further details.
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
    /**< Primitive list during BVH construction. */
    std::unique_ptr<Bvh_Primitive[]>        m_bvhpri = nullptr;
    /**< Root node of the BVH structure. */
    std::unique_ptr<Bvh_Node>               m_root = nullptr;
    /**< Maximum primitives in a leaf node. During BVH construction, a node with less primitives will be marked as a leaf node. */
    unsigned                                m_maxPriInLeaf = 8;
    /**< Maximum depth of node in BVH. */
    unsigned                                m_maxNodeDepth = 16;

    //! @brief Split current BVH node.
    //!
    //! @param node         The BVH node to be split.
    //! @param start        The start offset of primitives that the node holds.
    //! @param end          The end offset of primitives that the node holds.
    //! @param depth        The current depth of the node. Starting from 1 for root node.
    void    splitNode( Bvh_Node* node , unsigned start , unsigned end , unsigned depth );

    //! @brief Mark the current node as leaf node.
    //!
    //! @param node         The BVH node to be marked as leaf node.
    //! @param start        The start offset of primitives that the node holds.
    //! @param end          The end offset of primitives that the node holds.
    void    makeLeaf( Bvh_Node* node , unsigned start , unsigned end );

    //! @brief A recursive function that traverses the BVH node.
    //!
    //! @param node         The root node of the (sub)tree to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The structure holding the intersection information. If empty pointer is passed,
    //!                     it will return as long as one intersection is found and it won't be necessary to be
    //!                     the nearest one.
    //! @param fmin         The minimum range along the ray.
    //! @return             True if there is intersection, otherwise it will return false.
    bool    traverseNode( const Bvh_Node* node , const Ray& ray , SurfaceInteraction* intersect , float fmin ) const;

    //! @brief A recursive helper function that traverse the BVH to find all intersections.
    //!
    //! @param node         The root node of the (sub)tree to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The result intersections.
    //! @param fmin         The minimum range along the ray, any intersection before it will be ignored.
    //! @param              Material ID to avoid if it is not invalid.
    void    traverseNode( const Bvh_Node* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , const StringID matID ) const;

    SORT_STATS_ENABLE( "Spatial-Structure(BVH)" )
};
