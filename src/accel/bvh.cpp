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

#include "bvh.h"
#include <algorithm>
#include "geometry/ray.h"
#include "log/log.h"
#include "managers/memmanager.h"
#include "geometry/intersection.h"

static const unsigned   BVH_LEAF_PRILIST_MEMID  = 1027;
static const unsigned   BVH_SPLIT_COUNT         = 16;
static const float      BVH_INV_SPLIT_COUNT     = 0.0625f;

IMPLEMENT_CREATOR( Bvh );

// destructor
Bvh::~Bvh()
{
	deallocMemory();
}

// output log information
void Bvh::OutputLog() const
{
    slog( LOG_LEVEL::INFO , LOG_TYPE::SPATIAL_ACCELERATOR , "Spatial accelerator type is BVH ( Bounding Volume Hierarchy )." );
    slog( LOG_LEVEL::DEBUG , LOG_TYPE::SPATIAL_ACCELERATOR , stringFormat( "Maximum depth in BVH tree is %d. Total number of nodes in it is %d, number of inner nodes is %d, number of leaf nodes is %d. Average number of triangles per leaf nodes is %f, maximum number of triangles in leaf nodes is %d" , m_bvhDepth , m_totalNode , m_totalNode - m_leafNode , m_leafNode , (((float)m_primitives->size())/m_leafNode) , m_maxPriInLeaf ) );
}

// malloc the memory
void Bvh::mallocMemory()
{
	SORT_PREMALLOC( (unsigned)(sizeof( Bvh_Primitive ) * m_primitives->size()) , BVH_LEAF_PRILIST_MEMID );
	m_bvhpri = SORT_MEMORY_ID( Bvh_Primitive , BVH_LEAF_PRILIST_MEMID );
}

// dealloc memory
void Bvh::deallocMemory()
{
	SORT_DEALLOC( BVH_LEAF_PRILIST_MEMID );
    deleteNode( m_root );
}

// build the acceleration structure
void Bvh::Build()
{
	// malloc memory
	mallocMemory();

	// build bounding box
	computeBBox();

	// generate bvh primitives
    for( auto primitive : *m_primitives )
        SORT_MALLOC_ID(Bvh_Primitive,BVH_LEAF_PRILIST_MEMID)(primitive);
    
	// recursively split node
    m_root = new Bvh_Node();
	splitNode( m_root , 0u , (unsigned)m_primitives->size() , 0u );
}

// recursively split BVH node
void Bvh::splitNode( Bvh_Node* node , unsigned _start , unsigned _end , unsigned depth )
{
	m_totalNode ++;
	m_bvhDepth = max( depth , m_bvhDepth );

	// generate the bounding box for the node
	for( unsigned i = _start ; i < _end ; i++ )
		node->bbox.Union( m_bvhpri[i].GetBBox() );

	unsigned tri_num = _end - _start;
	if( tri_num <= m_maxPriInLeaf ){
		makeLeaf( node , _start , _end );
		return;
	}

	// pick best split plane
	unsigned split_axis;
	float split_pos;
	float sah = pickBestSplit( split_axis , split_pos , node , _start , _end );
	if( sah >= tri_num ){
		makeLeaf( node , _start , _end );
		return;
	}

	// partition the data
    auto compare = [split_pos,split_axis](const Bvh::Bvh_Primitive& pri){return pri.m_centroid[split_axis] < split_pos;};
	const Bvh_Primitive* middle = partition( &m_bvhpri[_start] , &m_bvhpri[_end-1]+1 , compare );
	unsigned mid = (unsigned)(middle - m_bvhpri);

    node->left = new Bvh_Node();
	splitNode( node->left , _start , mid , depth + 1 );

    node->right = new Bvh_Node();
	splitNode( node->right , mid , _end , depth + 1 );
}

// pick best split plane among all possible splits
float Bvh::pickBestSplit( unsigned& axis , float& split_pos , Bvh_Node* node , unsigned _start , unsigned _end )
{
	BBox inner;
	for( unsigned i = _start ; i < _end ; i++ )
		inner.Union( m_bvhpri[i].m_centroid );

	unsigned tri_num = _end - _start;
	axis = inner.MaxAxisId();
	float min_sah = FLT_MAX;

	// distribute the triangles into bins
	unsigned	bin[BVH_SPLIT_COUNT];
	BBox		bbox[BVH_SPLIT_COUNT];
	BBox		rbox[BVH_SPLIT_COUNT-1];
	memset( bin , 0 , sizeof( unsigned ) * BVH_SPLIT_COUNT );
	float split_start = inner.m_Min[axis];
	float split_delta = inner.Delta(axis) * BVH_INV_SPLIT_COUNT;
	float inv_split_delta = 1.0f / split_delta;
    for( unsigned i = _start ; i < _end ; i++ ){
		int index = (int)((m_bvhpri[i].m_centroid[axis] - split_start) * inv_split_delta);
		index = min( index , (int)(BVH_SPLIT_COUNT - 1) );
		bin[index]++;
		bbox[index].Union( m_bvhpri[i].GetBBox() );
	}

	rbox[BVH_SPLIT_COUNT-2].Union( bbox[BVH_SPLIT_COUNT-1] );
	for( int i = BVH_SPLIT_COUNT-3; i >= 0 ; i-- )
		rbox[i] = Union( rbox[i+1] , bbox[i+1] );

	unsigned	left = bin[0];
	BBox		lbox = bbox[0];
	float pos = split_delta + split_start ;
	for( unsigned i = 0 ; i < BVH_SPLIT_COUNT - 1 ; i++ ){
		float sah_value = sah( left , tri_num - left , lbox , rbox[i] , node->bbox );
		if( sah_value < min_sah ){
			min_sah = sah_value;
			split_pos = pos;
		}
		left += bin[i+1];
		lbox.Union( bbox[i+1] );
		pos += split_delta;
	}

	return min_sah;
}

// evaluate sah value
float Bvh::sah( unsigned left , unsigned right , const BBox& lbox , const BBox& rbox , const BBox& box )
{
	return (left * lbox.HalfSurfaceArea() + right * rbox.HalfSurfaceArea()) / box.HalfSurfaceArea();
}

// make the node as a leaf
void Bvh::makeLeaf( Bvh_Node* node , unsigned _start , unsigned _end )
{
	node->pri_num = _end - _start;
	node->pri_offset = _start;

	m_leafNode++;
	m_maxLeafTriNum = max( m_maxLeafTriNum , node->pri_num );
}

// get the intersection between the ray and the primitive set
bool Bvh::GetIntersect( const Ray& ray , Intersection* intersect ) const
{
	float fmax;
	float fmin = Intersect( ray , m_bbox , &fmax );
	if( fmin < 0.0f )
		return false;

	if( traverseNode( m_root , ray , intersect , fmin , fmax ) ){
		if( intersect == 0 )
			return true;
		return intersect->primitive != 0 ;
	}
    
	return false;
}

// traverse node
bool Bvh::traverseNode( const Bvh_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const
{
	if( fmin < 0.0f )
		return false;

	if( intersect && intersect->t < fmin )
		return true;
	
    if( node->pri_num != 0 ){
        unsigned _start = node->pri_offset;
        unsigned _tri = node->pri_num;
        unsigned _end = _start + _tri;
        
        bool inter = false;
        for( unsigned i = _start ; i < _end ; i++ ){
            inter |= m_bvhpri[i].primitive->GetIntersect( ray , intersect );
            if( intersect == 0 && inter )
                return true;
        }
        return inter;
	}

    const Bvh_Node* left = node->left;
	const Bvh_Node* right = node->right;

	float	_fmin0 , _fmax0;
	_fmin0 = Intersect( ray , left->bbox , &_fmax0 );
	float	_fmin1 , _fmax1;
	_fmin1 = Intersect( ray , right->bbox , &_fmax1 );

	bool inter = false;
	if( _fmin1 > _fmin0 ){
		inter |= traverseNode( left , ray , intersect , _fmin0 , _fmax0 );
		if( inter && intersect == 0 ) return true;
		inter |= traverseNode( right , ray , intersect , _fmin1 , _fmax1 );
	}else{
		inter |= traverseNode( right , ray , intersect , _fmin1 , _fmax1 );
		if( inter && intersect == 0 ) return true;
		inter |= traverseNode( left , ray , intersect , _fmin0 , _fmax0 );
	}
	if( intersect == 0 )
		return inter;
	return true;
}

// delete bvh node recursively
void Bvh::deleteNode( Bvh_Node* node ){
    if( !node )
        return;
    deleteNode( node->left );
    deleteNode( node->right );
    delete node;
}
