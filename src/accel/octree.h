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
#include "geometry/bbox.h"

//! @brief OcTree
/**
 * OcTree is a popular data strucutre in scene management, which is commonly seen in game engines.
 * Instead of scene visibility management, it can also serves for the purpose of accelerating ray
 * tracer applications.
 */
class OcTree : public Accelerator
{
public:
	DEFINE_CREATOR( OcTree , Accelerator , "octree" );

	//! destructor
	~OcTree();

    //! @brief Get intersection between the ray and the primitive set using OcTree.
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
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	//! Build the OcTree in O(Nlg(N)) time
	virtual void Build();

    //! OcTree node structure
    struct OcTreeNode{
        OcTreeNode*					    child[8] = {nullptr};	/**< Child node pointers, all will be NULL if current node is a leaf.*/
        std::vector<const Primitive*>	primitives;             /**< Primitives buffer.*/
        BBox						    bb;                     /**< Bounding box for this octree node.*/
    };
    
    //! @brief Primitive information in octree node.
    struct NodePrimitiveContainer{
        std::vector<const Primitive*>	primitives;		/**< Primitive buffer used during octree construction.*/
    };
    
private:
	OcTreeNode*	m_pRoot = nullptr;				/**< Pointer to the root node of this octree.*/
	const unsigned	m_uMaxPriInLeaf = 16;		/**< Maximum number of primitives allowed in a leaf node, 16 is the default value.*/
	const unsigned	m_uMaxDepthInOcTree = 16;	/**< Maximum depth of the octree, 16 is the default value.*/

	//! @brief Split current node into eight if criteria is not met. Otherwise, it will make it a leaf.\n
	//! This function invokes itself recursively, so the whole sub-tree will be built once it is called.
	//! @param node         Node to be splitted.
	//! @param container    Container holding all primitive information in this node.
	//! @param bb           Bounding box of this node.
	//! @param depth        Current depth of this node.
	void splitNode( OcTreeNode* node , NodePrimitiveContainer* container , unsigned depth );

	//! @brief Making the current node as a leaf node.
	//! An new index buffer will be allocated in this node.
	//! @param node         Node to be made as a leaf node.
	//! @param container    Container holdes all primitive information in this node.
	void makeLeaf( OcTreeNode* node , NodePrimitiveContainer* container );

	//! @brief Traverse OcTree recursively and return if there is interesection.
	//! @param node         Sub-tree belongs to this node will be visited in a depth first manner.
	//! @param ray          The input ray to be tested.
    //! @param intersect    A pointer to the result intersection information. If empty is passed, it will return as long as an intersection is
    //!                     detected and it is not necessarily to be the nearest one.
	//! @param fmin         Current minimum value along the ray
	//! @param fmax         Current maximum value along the ray.
	//! @return             Whether the ray intersects anything in the primitive set
	bool traverseOcTree( const OcTreeNode* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;

	//! @brief Release OcTree memory.
	//! @param node Sub-tree belongs to this node will be released recursively.
	void releaseOcTree( OcTreeNode* node );
    
    SORT_STATS_ENABLE( "Spatial-Structure(OcTree)" )
};
