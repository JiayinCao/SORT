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

//! @brief OcTree
/**
 * OcTree is a popular data structure in scene management, which is commonly seen in game engines.
 * Instead of scene visibility management, it can also serves for the purpose of accelerating ray
 * tracer applications.
 */
class OcTree : public Accelerator{
    //! @brief      OcTree node structure
    struct OcTreeNode{
        /**< Child node pointers, all will be NULL if current node is a leaf.*/
        std::unique_ptr<OcTreeNode>     child[8] = {nullptr};
        /**< Primitives buffer.*/
        std::vector<const Primitive*>	primitives;
        /**< Bounding box for this OcTree node.*/
        BBox						    bb;
    };

public:
    DEFINE_CREATOR( OcTree , Accelerator , "OcTree" );

    //! @brief      Get intersection between the ray and the primitive set using KD-Tree.
    //!
    //! It will return true if there is intersection between the ray and the primitive 
    //! set. In case of an existed intersection, if intersect is not empty, it will fill 
    //! the structure and return the nearest intersection.
    //! If intersect is nullptr, it will stop as long as one intersection is found, it 
    //! is not necessary to be the nearest one.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided, 
    //!                     it stops as long as it finds an intersection. It is faster 
    //!                     than the one with intersection information data and suitable 
    //!                     for shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise 
    //!                     it returns false.
    bool GetIntersect( const Ray& r , Intersection* intersect ) const override;

	//! Build the OcTree in O(Nlg(N)) time.
    //!
    //! @param scene    The rendering scene.
	void    Build(const Scene& scene) override;
    
    //! @brief      Primitive information in OcTree node.
    struct NodePrimitiveContainer{
        /**< Primitive buffer used during OcTree construction.*/
        std::vector<const Primitive*>	primitives;
    };

    //! @brief      Serializing data from stream.
    //!
    //! @param      Stream where the serialization data comes from. Depending on different 
    //!             situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override{
        stream >> m_maxDepthInOcTree;
        stream >> m_maxPriInLeaf;
    }

private:
    /**< Pointer to the root node of this OcTree.*/
	std::unique_ptr<OcTreeNode>	m_root = nullptr;
    /**< Maximum number of primitives allowed in a leaf node, 16 is the default value.*/
	unsigned	m_maxPriInLeaf = 16;
    /**< Maximum depth of the OcTree, 16 is the default value.*/
	unsigned	m_maxDepthInOcTree = 16;

	//! @brief  Split current node into eight if criteria is not met. Otherwise, it will make it a leaf.
    //!
	//! This function invokes itself recursively, so the whole sub-tree will be built 
    //! once it is called.
    //!
	//! @param node         Node to be slitted.
	//! @param container    Container holding all primitive information in this node.
	//! @param bb           Bounding box of this node.
	//! @param depth        Current depth of this node.
	void splitNode( OcTreeNode* node , NodePrimitiveContainer* container , unsigned depth );

	//! @brief Making the current node as a leaf node.
	//! An new index buffer will be allocated in this node.
	//! @param node         Node to be made as a leaf node.
	//! @param container    Container holding all primitive information in this node.
	void makeLeaf( OcTreeNode* node , NodePrimitiveContainer* container );

	//! @brief  Traverse OcTree recursively and return if there is intersection.
    //!
	//! @param node         Sub-tree belongs to this node will be visited in a depth first manner.
	//! @param ray          The input ray to be tested.
    //! @param intersect    A pointer to the result intersection information. If empty is passed, 
    //!                     it will return as long as an intersection is detected and it is not 
    //!                     necessarily to be the nearest one.
	//! @param fmin         Current minimum value along the ray
	//! @param fmax         Current maximum value along the ray.
	//! @return             Whether the ray intersects anything in the primitive set
	bool traverseOcTree( const OcTreeNode* node , const Ray& ray , Intersection* intersect , 
                         float fmin , float fmax ) const;

	//! @brief  Release OcTree memory.
    //!
	//! @param node     Sub-tree belongs to this node will be released recursively.
	void releaseOcTree( OcTreeNode* node );
    
    SORT_STATS_ENABLE( "Spatial-Structure(OcTree)" )
};
