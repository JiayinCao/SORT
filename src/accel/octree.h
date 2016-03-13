/*
   FileName:      octree.h

   Created Time:  2016-3-7

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_OCTREE
#define	SORT_OCTREE

#include "accelerator.h"
#include "geometry/bbox.h"

//! OcTree node structure
struct OcTreeNode{
	OcTreeNode*					child[8];	/**< Child node pointers, all will be NULL if current node is a leaf.*/
	vector<const Primitive*>	primitives;	/**< Primitives buffer.*/
	BBox						bb;			/**< Bounding box for this octree node.*/

	//! A container sets everything with 0.
	OcTreeNode(){
		memset( child , 0 , sizeof(child) );
	}
};

//! @brief Triangle information in octree node.
struct NodeTriangleContainer{
	vector<const Primitive*>	primitives;		/**< Primitive buffer used during octree construction.*/

	//! Release list memory in destructor.
	~NodeTriangleContainer(){
		primitives.clear();
	}
};

/////////////////////////////////////////////////////////////////////////////////////
//	definition of OcTree
class OcTree : public Accelerator
{
// public method
public:
	DEFINE_CREATOR( OcTree , "octree" );

	//! default construction
	OcTree();
	//! destructor
	~OcTree();

	//! Get the intersection between the ray and the primitive set
	//! @param r The ray
	//! @param intersect The intersection result
	//! @return 'true' if the ray pirece one of the primitve in the list
	virtual bool GetIntersect( const Ray& r , Intersection* intersect ) const;

	//! Build the OcTree in O(Nlg(N)) time
	virtual void Build();

	//! output log information
	void OutputLog() const;

// private field
private:
	OcTreeNode*	m_pRoot;				/**< Pointer to the root node of this octree.*/
	unsigned	m_uMaxTriInLeaf;		/**< Maximum number of triangles allowed in a leaf node, 16 is the default value.*/
	unsigned	m_uMaxDepthInOcTree;	/**< Maximum depth of the octree, 16 is the default value.*/

	//! Split current node into eight if criteria is not met. Otherwise, it will make it a leaf.\n
	//! This function invokes itself recursively, so the whole sub-tree will be built once it is called.
	//! @param node Node to be splitted.
	//! @param container Container holdes all triangle information in this node.
	//! @param bb Bounding box of this node.
	//! @param depth Current depth of this node.
	void splitNode( OcTreeNode* node , NodeTriangleContainer* container , unsigned depth );

	//! Making the current node as a leaf node.
	//! An new index buffer will be allocated in this node.
	//! @param node Node to be made as a leaf node.
	//! @param container Container holdes all triangle information in this node.
	void makeLeaf( OcTreeNode* node , NodeTriangleContainer* container );

	//! Traverse OcTree recursively.
	//! @param node Sub-tree belongs to this node will be visited in a depth first manner.
	//! @param ray The input ray to be tested.
	//! @param fmin Current minimum value along the range.
	//! @param fmax Current maximum value along the range.
	//! @result Whether the ray intersects anything in the primitive set
	bool traverseOcTree( const OcTreeNode* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const;

	//! Release OcTree memory.
	//! @param node Sub-tree belongs to this node will be released recursively.
	void releaseOcTree( OcTreeNode* node );
};

#endif