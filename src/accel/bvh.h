/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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
#include "geometry/primitive.h"

//! @brief Bounding volume hierarchy.
/**
 * BVH(Bounding volume hierarchy) is a classic spatial acceleration structure commonly
 * used in ray tracing applications. This is a sah BVH implementation whose construction
 * is in O(N*lg(N)).
 * Please refer to this paper 
 * <a href="http://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf">
 * On fast Construction of SAH-based Bounding Volume Hierarchies</a> for further details.
 */
class Bvh : public Accelerator
{
public:
	DEFINE_CREATOR( Bvh , Accelerator , "bvh" );

	//! Destructor
    ~Bvh() override;

    //! @brief Get intersection between the ray and the primitive set using BVH.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an existed intersection, if intersect is not empty, it will fill the
    //! structure and return the nearest intersection.
    //! If intersect is nullptr, it will stop as long as one intersection is found, it is not
    //! necessary to be the nearest one.
    //! False will be returned if there is no intersection at all.
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, it stops as
    //!                     long as it finds an intersection. It is faster than the one with intersection information
    //!                     data and suitable for shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise it returns false.
    bool GetIntersect( const Ray& r , Intersection* intersect ) const override;

    //! Build BVH structure in O(N*lg(N)).
	void Build() override;

    //! Bounding volume hierarchy node.
    struct Bvh_Node
    {
        BBox		bbox;               /**< Bounding box of the BVH node. */
        unsigned 	pri_num     = 0;    /**< Number of primitives in the BVH node. */
        unsigned	pri_offset  = 0;    /**< Offset in the primitive buffer. It is 0 for interior nodes. */
        Bvh_Node*   left        = 0;    /**< Left child of the BVH node. */
        Bvh_Node*   right       = 0;    /**< Right child of the BVH node. */
    };
    
    //! Bounding volume hierarchy node primitives. It is used during BVH construction.
    struct Bvh_Primitive
    {
        Primitive*	primitive;      /**< Primitive lists for this node. */
        Point		m_centroid;     /**< Center point of the BVH node. */
        
        //! @brief Constructor of Bvh_Primitive.
        //! @param p primitive list holding all primitives in the node.
        Bvh_Primitive( Primitive* p ):primitive(p)
        {m_centroid = ( p->GetBBox().m_Max + p->GetBBox().m_Min ) * 0.5f;}
        
        //! Get bounding box of this primitive set.
        //! @return Axis-Aligned bounding box holding all the primitives.
        const BBox& GetBBox() const
        {return primitive->GetBBox();}
    };
    
private:
    Bvh_Primitive*	m_bvhpri = nullptr; /**< Primitive list during BVH construction. */
    Bvh_Node*       m_root = nullptr;   /**< Root node of the BVH structure. */

	const unsigned	m_maxPriInLeaf = 8; /**< Maximum primitives in a leaf node. During BVH construction, a node with less primitives will be marked as a leaf node. */

	//! Malloc necessary memory.
	void mallocMemory();

	//! Dealloc all allocated memory.
	void deallocMemory();

	//! @brief Split current BVH node.
    //! @param node     The BVH node to be split.
    //! @param _start   The start offset of primitives that the node holds.
    //! @param _end     The end offset of prititives that the node holds.
    //! @param depth    The current depth of the node.
	void splitNode( Bvh_Node* node , unsigned _start , unsigned _end , unsigned depth );

	//! @brief Mark the current node as leaf node.
    //! @param node     The BVH node to be marked as leaf node.
    //! @param _start   The start offset of primitives that the node holds.
    //! @param _end     The end offset of prititives that the node holds.
	void makeLeaf( Bvh_Node* node , unsigned _start , unsigned _end );

	//! @brief Evaluate the SAH value of a specific splitting.
    //! @param left     The number of primitives in the left node to be split.
    //! @param right    The number of primitives in the right node to be split.
    //! @param lbox     Bounding box of the left node to be split.
    //! @param rbox     Bounding box of the right node to be split.
    //! @param box      Bounding box of the current node.
    //! @return         SAH value of the specific split plane.
	float sah( unsigned left , unsigned right , const BBox& lbox , const BBox& rbox , const BBox& box );

	//! @brief Pick the best split among all possible splits.
    //! @param axis      The selected axis id of the picked split plane.
    //! @param split_pos Position of the selected split plane.
    //! @param node      The node to be split.
    //! @param _start    The start offset of primitives that the node holds.
    //! @param _end      The end offset of prititives that the node holds.
    //! @return          The SAH value of the selected best split plane.
	float pickBestSplit( unsigned& axis , float& split_pos , Bvh_Node* node , unsigned _start , unsigned _end );

	//! @brief A recursive function that traverses the BVH node.
    //! @param node         The node to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The structure holding the intersection information. If empty pointer is passed,
    //!                     it will return as long as one intersection is found and it won't be necessary to be
    //!                     the nearest one.
    //! @param fmin         The minimum range along the ray.
    //! @param fmax         The maximum range along the ray.
    //! @return             True if there is intersection, otherwise it will return false.
	bool traverseNode( const Bvh_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;
    
    //! @brief Delete all nodes in the BVH.
    //! @param node The node to be deleted.
    void deleteNode( Bvh_Node* node );
    
    SORT_STATS_ENABLE( "Spatial-Structure(BVH)" )
};
