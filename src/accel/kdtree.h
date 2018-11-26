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

//! @brief K-Dimensional Tree or KD-Tree.
/**
 * A KD-Tree is a spatial partitioning data structure for organizing primitives in
 * a k-dimensional space. In the context of a ray tracer, k usually equals to 3.
 * KD-Tree is a very popular spatial data structure for accelerating ray tracing
 * algorithms and it is also one of the most efficient ones.
 * The construction of this KD-Tree works in O(N*lg(N)), which is proved to be the
 * optimal solution in one single thread.
 * Please refer to this paper
 * <a href="http://www.eng.utah.edu/~cs6965/papers/kdtree.pdf">
 * On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)</a> for further details.
 */
class KDTree : public Accelerator
{
public:
	DEFINE_CREATOR( KDTree , Accelerator , "kd_tree" );

	//! Destructor that delete all allocated KD-Tree memory.
    ~KDTree() override;

    //! @brief Get intersection between the ray and the primitive set using KD-Tree.
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

	//! Build KD-Tree structure in O(N*lg(N)).
	void Build() override;

    //! KD-Tree split plane type
    enum class Split_Type
    {
        Split_None = 0,     /**< An invalid type. */
        Split_Start = 1,    /**< Split plane at the start of one primitive along an axis. */
        Split_End = 2,      /**< Split plane at the end of one primitive along an axis. */
    };
    
    //! KD-Tree node structure
    struct Kd_Node
    {
    public:
        Kd_Node*					    leftChild = nullptr;	/**< Pointer to the left child of the KD-Tree node. */
        Kd_Node*					    rightChild = nullptr;	/**< Pointer to the right child of the KD-Tree node. */
        BBox						    bbox;                   /**< Bounding box of the KD-Tree node. */
        std::vector<const Primitive*>	primitivelist;          /**< Vector holding all primitives in the node. It 
                                                                should be empty for interior nodes. */
        unsigned					    flag = 0;               /**< Special mask used for nodes. The node is a leaf node if it is 3. 
                                                                For interior nodes, it will be the cooreponding id of the split axis.*/
        float						    split = 0.0f;           /**< Split position */
        
        //! @brief Constructor taking a bounding box.
        //! @param bb Bounding box of the node.
        Kd_Node( const BBox& bb ):bbox(bb){}
    };
    
    //! A split candidate.
    struct Split
    {
        float		pos = 0.0f;                     /**< Position of the split plane along a specific axis. */
        Split_Type	type = Split_Type::Split_None;  /**< The type of the split plane. */
        unsigned	id = 0;                         /**< The index of the pritmitive that triggers the split plane in the primitive list.*/
        Primitive*	primitive = nullptr;            /**< The pointer pointing to the primitive that triggers the split plane.*/
        
        //! Constructor of Split.
        //! @param po   Position of the split plane.
        //! @param t    Type of the split plane.
        //! @param pid  Index of the primitive that triggers the split plane.
        //! @param p    The pointer to the primitive that triggers the split plane.
        Split( float po = 0.0f , Split_Type t = Split_Type::Split_None, unsigned pid = 0, Primitive* p = nullptr): pos(po) , type(t) , id(pid) , primitive(p){
        }
        
        //! Comparator for the struct.
        //! @param split    The split plane to compare with.
        //! @return         A comparing result based on position and type of the split planes.
        bool operator < ( const Split& split ) const
        {
            if( pos != split.pos )
                return pos<split.pos;
            return type < split.type ;
        }
    };
    
    //! The structure holds all possible split plane during KD-Tree construction.
    struct Splits
    {
        Split*		split[3] = { nullptr , nullptr , nullptr }; /**< Split planes along three different axis. */
        
        //! Release the allcoated memory.
        void Release()
        {
            SAFE_DELETE_ARRAY(split[0]);
            SAFE_DELETE_ARRAY(split[1]);
            SAFE_DELETE_ARRAY(split[2]);
        }
    };
    
private:
	Kd_Node*		m_root = nullptr;           /**< Root node of the KD-Tree. */
	unsigned char*	m_temp = nullptr;           /**< Temporary buffer for marking primitives. */

	const unsigned	m_maxDepth = 28;            /**< Maximum allowed depth of KD-Tree. */
	const unsigned	m_maxPriInLeaf = 32;        /**< Maximum allowed number of primitives in a leaf node. */

    //! @brief Split current KD-Tree node.
    //! @param node     The KD-Tree node to be split.
    //! @param splits   The split plane that holds all primitive pointers.
    //! @param prinum   The number of primitives in the node.
    //! @param depth    The current depth of the node.
	void splitNode( Kd_Node* node , Splits& splits , unsigned prinum , unsigned depth );

    //! @brief Evaluate SAH value for a specific split plane.
    //! @param l        Number of primitives on the left of the split plane.
    //! @param r        Number of primitives on the right of the split plane.
    //! @param axis     ID of splitting axis.
    //! @param split    Position along the splitting axis of the split plane.
    //! @param box      Bounding box of the KD-Tree node.
    //! @return         The Evaluated SAH value for the split.
	float sah( unsigned l , unsigned r , unsigned axis , float split , const BBox& box );
	
    //! @brief Pick the split plane with minimal SAH value.
    //! @param splits       Split information that holds all possible split plane information.
    //! @param prinum       Number of all primitives in the current node.
    //! @param box          Axis aligned bounding box of the node.
    //! @param splitAxis    ID of the splitting axis.
    //! @param split_offset ID of the best split plane that is picked.
    //! @return         The SAH value of the selected split that has the minimal SAH value.
	float pickSplitting( const Splits& splits , unsigned prinum , const BBox& box ,
						 unsigned& splitAxis , unsigned& split_offset );
	
    //! @brief Mark the current node as leaf node.
    //! @param node     The KD-Tree node to be marked as leaf node.
    //! @param splits   Split plane information that holds all primitive pointers.
    //! @param prinum   The number of primitives in the node.
	void makeLeaf( Kd_Node* node , Splits& splits , unsigned prinum );
	
    //! @brief A recursive function that traverses the KD-Tree node.
    //! @param node         The node to be traversed.
    //! @param ray          The ray to be tested.
    //! @param intersect    The structure holding the intersection information. If empty pointer is passed,
    //!                     it will return as long as one intersection is found and it won't be necessary to be
    //!                     the nearest one.
    //! @param fmin         The minimum range along the ray.
    //! @param fmax         The maximum range along the ray.
    //! @return             True if there is intersection, otherwise it will return false.
	bool traverse( const Kd_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;

    //! @brief Delete all sub tree originating from node.
    //! @param node The KD-Tree node to be deleted.
	void deleteKdNode( Kd_Node* node );
    
    SORT_STATS_ENABLE( "Spatial-Structure(KDTree)" )
};
