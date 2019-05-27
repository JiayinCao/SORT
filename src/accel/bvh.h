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

#pragma once

#include "accelerator.h"
#include "core/primitive.h"

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
    bool    GetIntersect( const Ray& r , Intersection* intersect ) const override;

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

    //! @brief Evaluate the SAH value of a specific splitting.
    //!
    //! @param left         The number of primitives in the left node to be split.
    //! @param right        The number of primitives in the right node to be split.
    //! @param lbox         Bounding box of the left node to be split.
    //! @param rbox         Bounding box of the right node to be split.
    //! @param box          Bounding box of the current node.
    //! @return             SAH value of the specific split plane.
    float   sah( unsigned left , unsigned right , const BBox& lbox , const BBox& rbox , const BBox& box );

    //! @brief Pick the best split among all possible splits.
    //!
    //! @param axis         The selected axis id of the picked split plane.
    //! @param split_pos    Position of the selected split plane.
    //! @param node         The node to be split.
    //! @param start        The start offset of primitives that the node holds.
    //! @param end          The end offset of primitives that the node holds.
    //! @return             The SAH value of the selected best split plane.
    float   pickBestSplit( unsigned& axis , float& split_pos , Bvh_Node* node , unsigned start , unsigned end );

    //! @brief A recursive function that traverses the BVH node.
    //!
    //! @param node         The root node of the (sub)tree to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The structure holding the intersection information. If empty pointer is passed,
    //!                     it will return as long as one intersection is found and it won't be necessary to be
    //!                     the nearest one.
    //! @param fmin         The minimum range along the ray.
    //! @return             True if there is intersection, otherwise it will return false.
    bool    traverseNode( const Bvh_Node* node , const Ray& ray , Intersection* intersect , float fmin ) const;

    SORT_STATS_ENABLE( "Spatial-Structure(BVH)" )
};
