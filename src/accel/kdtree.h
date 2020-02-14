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

//! @brief K-Dimensional Tree or KD-Tree.
/**
 * A KD-Tree is a spatial partitioning data structure for organizing primitives in a
 * k-dimensional space. In the context of a ray tracer, k usually equals to 3. KD-Tree
 * is a very popular spatial data structure for accelerating ray tracing algorithms
 * and it is also one of the most efficient ones.
 */
class KDTree : public Accelerator{
    //! @brief KD-Tree split plane type
    enum class Split_Type {
        Split_None = 0,     /**< An invalid type. */
        Split_Start = 1,    /**< Split plane at the start of one primitive along an axis. */
        Split_End = 2,      /**< Split plane at the end of one primitive along an axis. */
    };

    //! @brief KD-Tree node structure.
    struct Kd_Node {
    public:
        /**< Pointer to the left child of the KD-Tree node. */
        std::unique_ptr<Kd_Node>                        leftChild = nullptr;
        /**< Pointer to the right child of the KD-Tree node. */
        std::unique_ptr<Kd_Node>                        rightChild = nullptr;
        /**< Bounding box of the KD-Tree node. */
        BBox                            bbox;
        /**< Vector holding all primitives in the node. It should be empty for interior nodes. */
        std::vector<const Primitive*>   primitivelist;
        /**< Special mask used for nodes. The node is a leaf node if it is 3. For interior
        nodes, it will be the corresponding id of the split axis.*/
        unsigned                        flag = 0;
        /**< Split position */
        float                           split = 0.0f;

        //! @brief Constructor taking a bounding box.
        //!
        //! @param bb   Bounding box of the node.
        Kd_Node(const BBox& bb) :bbox(bb) {}
    };

    //! @brief  A split candidate.
    struct Split {
        /**< Position of the split plane along a specific axis. */
        float       pos = 0.0f;
        /**< The type of the split plane. */
        Split_Type  type = Split_Type::Split_None;
        /**< The index of the primitive that triggers the split plane in the primitive list.*/
        unsigned    id = 0;
        /**< The pointer pointing to the primitive that triggers the split plane.*/
        const Primitive*  primitive = nullptr;

        //! @brief  Constructor of Split.
        //!
        //! @param po   Position of the split plane.
        //! @param t    Type of the split plane.
        //! @param pid  Index of the primitive that triggers the split plane.
        //! @param p    The pointer to the primitive that triggers the split plane.
        Split(float po = 0.0f, Split_Type t = Split_Type::Split_None, unsigned pid = 0, const Primitive* p = nullptr) :
            pos(po), type(t), id(pid), primitive(p) {
        }

        //! Comparator for the struct.
        //!
        //! @param split    The split plane to compare with.
        //! @return         A comparing result based on position and type of the split planes.
        bool operator < (const Split& split) const{
            if (pos != split.pos)
                return pos<split.pos;
            return type < split.type;
        }
    };

    //! @brief  The structure holds all possible split plane during KD-Tree construction.
    struct Splits {
        /**< Split planes along three different axis. */
        std::unique_ptr<Split[]>        split[3] = { nullptr , nullptr , nullptr };
    };

public:
    DEFINE_RTTI( KDTree , Accelerator );

    //! @brief Get intersection between the ray and the primitive set using KD-Tree.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an intersection, it will fill the structure and return the nearest intersection.
    //! This intersection could possibly be a fully transparent intersection, it is up to the higher
    //! level logic to handle (semi)transparency later.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result.
    //! @return             It will return true if there is an intersection, otherwise
    //!                     it returns false.
    bool GetIntersect( const Ray& r , SurfaceInteraction& intersect ) const override;

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
    //! The intersection returned doesn't guarrantee the order of the intersection of the results, but it does guarrantee to get the
    //! nearest N intersections.
    //! WARNING, it is quite possible to find an intersection that is fully transparent and still taking consideration of the light
    //! coming from that point. This is not strictly correct, but I would choose to live with it because it is not worth the extra
    //! performance overhead to fix the problem since it is very minor.
    //!
    //! @param  r           The input ray to be tested.
    //! @param  intersect   The intersection result that holds all intersectionn.
    //! @param  matID       We are only interested in intersection with the same material, whose material id should be set to matID.
    void GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID = INVALID_SID ) const override;

    //! @brief Build KD-Tree structure in O(N*lg(N)).
    //!
    //! The construction of this KD-Tree works in O(N*lg(N)), which is proved to be the
    //! optimal solution in one single thread.
    //! Please refer to this paper <a href = "http://www.eng.utah.edu/~cs6965/papers/kdtree.pdf">
    //! On building fast KD-Trees for Ray Tracing, and on doing that in O(N log N)</a>
    //! for further details.
    //!
    //! @param primitives       A vector holding all primitives.
    void    Build(const std::vector<const Primitive*>& primitives) override;

    //! @brief      Serializing data from stream.
    //!
    //! @param      Stream where the serialization data comes from. Depending on different
    //!             situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override{
        stream >> m_maxDepth;
        stream >> m_maxPriInLeaf;
    }

private:
    /**< Root node of the KD-Tree. */
    std::unique_ptr<Kd_Node>        m_root = nullptr;

    /**< Maximum allowed depth of KD-Tree. */
    unsigned        m_maxDepth = 28;
    /**< Maximum allowed number of primitives in a leaf node. */
    unsigned        m_maxPriInLeaf = 8;

    //! @brief  Split current KD-Tree node.
    //!
    //! @param node         The KD-Tree node to be split.
    //! @param splits       The split plane that holds all primitive pointers.
    //! @param prinum       The number of primitives in the node.
    //! @param depth        The current depth of the node.
    //! @param tmp          Temporary buffer for marking primitives.
    void splitNode( Kd_Node* node , Splits& splits , unsigned prinum , unsigned depth , unsigned char* tmp );

    //! @brief  Evaluate SAH value for a specific split plane.
    //!
    //! @param l            Number of primitives on the left of the split plane.
    //! @param r            Number of primitives on the right of the split plane.
    //! @param axis         ID of splitting axis.
    //! @param split        Position along the splitting axis of the split plane.
    //! @param box          Bounding box of the KD-Tree node.
    //! @return             The Evaluated SAH value for the split.
    float sah( unsigned l , unsigned r , unsigned axis , float split , const BBox& box );

    //! @brief  Pick the split plane with minimal SAH value.
    //!
    //! @param splits       Split information that holds all possible split plane information.
    //! @param prinum       Number of all primitives in the current node.
    //! @param box          Axis aligned bounding box of the node.
    //! @param splitAxis    ID of the splitting axis.
    //! @param split_offset ID of the best split plane that is picked.
    //! @return             The SAH value of the selected split that has the minimal
    //!                     SAH value.
    float pickSplitting( const Splits& splits , unsigned prinum , const BBox& box ,
                         unsigned& splitAxis , unsigned& split_offset );

    //! @brief  Mark the current node as leaf node.
    //!
    //! @param node     The KD-Tree node to be marked as leaf node.
    //! @param splits   Split plane information that holds all primitive pointers.
    //! @param prinum   The number of primitives in the node.
    void makeLeaf( Kd_Node* node , Splits& splits , unsigned prinum );

    //! @brief  A recursive function that traverses the KD-Tree node.
    //!
    //! @param node         The node to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The structure holding the intersection information. If empty
    //!                     pointer is passed, it will return as long as one intersection
    //!                     is found and it won't be necessary to be the nearest one.
    //! @param fmin         The minimum range along the ray.
    //! @param fmax         The maximum range along the ray.
    //! @return             True if there is intersection, otherwise it will return false.
    bool traverse( const Kd_Node* node , const Ray& ray , SurfaceInteraction* intersect , float fmin , float fmax ) const;

    //! @brief  A recursive function that traverses the KD-Tree node.
    //!
    //! @param node         The node to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The data structure holds all intersections.
    //! @param fmin         The minimum range along the ray.
    //! @param fmax         The maximum range along the ray.
    //! @param matID        Material ID to avoid if it is not invalid.
    void traverse( const Kd_Node* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , float fmax , const StringID matID ) const;

    //! @brief  Delete all sub tree originating from node.
    //!
    //! This function will recursively delete the whole (sub)tree of the node.
    //!
    //! @param node         The KD-Tree node to be deleted.
    void deleteKdNode( Kd_Node* node );

    SORT_STATS_ENABLE( "Spatial-Structure(KDTree)" )
};
