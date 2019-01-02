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

#include <string.h>
#include <algorithm>
#include "bvh.h"
#include "math/ray.h"
#include "managers/memmanager.h"
#include "math/intersection.h"

IMPLEMENT_CREATOR(Bvh);

SORT_STATS_DEFINE_COUNTER(sBvhNodeCount)
SORT_STATS_DEFINE_COUNTER(sBvhLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sBVHDepth)
SORT_STATS_DEFINE_COUNTER(sBvhMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sBvhPrimitiveCount)
SORT_STATS_DEFINE_COUNTER(sBvhLeafNodeCountCopy)

SORT_STATS_COUNTER("Spatial-Structure(BVH)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(BVH)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(BVH)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(BVH)", "Node Count", sBvhNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(BVH)", "Leaf Node Count", sBvhLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(BVH)", "BVH Depth", sBVHDepth);
SORT_STATS_COUNTER("Spatial-Structure(BVH)", "Maximum Primitive in Leaf", sBvhMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(BVH)", "Average Primitive Count in Leaf", sBvhPrimitiveCount , sBvhLeafNodeCountCopy );
SORT_STATS_AVG_COUNT("Spatial-Structure(BVH)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

static const unsigned   BVH_LEAF_PRILIST_MEMID  = 1027;
static const unsigned   BVH_SPLIT_COUNT         = 16;
static const float      BVH_INV_SPLIT_COUNT     = 0.0625f;

void Bvh::Build(const Scene& scene){
    SORT_PROFILE("Build Bvh");

    m_primitives = scene.GetPrimitives();
    
    // This memory allocation will be automatically deleted, meaning BVH class won't be responsible for clearing this memory.
    SORT_PREMALLOC((unsigned)(sizeof(Bvh_Primitive) * m_primitives->size()), BVH_LEAF_PRILIST_MEMID);
    m_bvhpri = SORT_MEMORY_ID(Bvh_Primitive, BVH_LEAF_PRILIST_MEMID);

	// build bounding box
	computeBBox();

	// generate BVH primitives
    for( auto& primitive : *m_primitives )
        SORT_MALLOC_ID(Bvh_Primitive,BVH_LEAF_PRILIST_MEMID)(primitive.get());
    
	// recursively split node
    m_root = std::make_unique<Bvh_Node>();
	splitNode( m_root.get() , 0u , (unsigned)m_primitives->size() , 1u );
    
	m_isValid = true;
	
    SORT_STATS(++sBvhNodeCount);
    SORT_STATS(sBvhLeafNodeCountCopy = sBvhLeafNodeCount);
}

bool Bvh::GetIntersect(const Ray& ray, Intersection* intersect) const{
    SORT_PROFILE("Traverse Bvh");
    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += intersect != nullptr);

    auto fmin = Intersect(ray, m_bbox);
    if (fmin < 0.0f)
        return false;

    if (traverseNode(m_root.get(), ray, intersect, fmin))
        return !intersect || intersect->primitive ;
    return false;
}

void Bvh::splitNode( Bvh_Node* node , unsigned start , unsigned end , unsigned depth ){
    SORT_STATS(sBVHDepth = std::max( sBVHDepth , (StatsInt)depth + 1 ) );
    
	// generate the bounding box for the node
	for( auto i = start ; i < end ; i++ )
		node->bbox.Union( m_bvhpri[i].GetBBox() );

	auto primitive_num = end - start;
	if( primitive_num <= m_maxPriInLeaf || depth == m_maxNodeDepth ){
		makeLeaf( node , start , end );
		return;
	}

	// pick best split plane
	unsigned    split_axis;
    float       split_pos;
    auto sah = pickBestSplit( split_axis , split_pos , node , start , end );
	if( sah >= primitive_num ){
		makeLeaf( node , start , end );
		return;
	}

	// partition the data
    auto compare = [split_pos,split_axis](const Bvh::Bvh_Primitive& pri){return pri.m_centroid[split_axis] < split_pos;};
    auto middle = std::partition( &m_bvhpri[start] , &m_bvhpri[end-1]+1 , compare );
    auto mid = (unsigned)(middle - m_bvhpri);

    // To avoid degenerated node that has nothing in it.
    // Technically, this shouldn't happen. Unlike KD-Tree implementation, there is only 16 split plane candidate, it is
    // totally possible to pick one with no primitive on one side of the plane, resulting a crash later during ray tracing.
    if (mid == start || mid == end){
        makeLeaf(node, start, end);
        return;
    }

    node->left = std::make_unique<Bvh_Node>();
	splitNode( node->left.get() , start , mid , depth + 1 );

    node->right = std::make_unique<Bvh_Node>();
	splitNode( node->right.get() , mid , end , depth + 1 );
    
    SORT_STATS(sBvhNodeCount+=2);
}

float Bvh::pickBestSplit( unsigned& axis , float& splitPos , Bvh_Node* node , unsigned start , unsigned end ){
	BBox inner;
	for(auto i = start ; i < end ; i++ )
		inner.Union( m_bvhpri[i].m_centroid );

    auto primitive_num = end - start;
	axis = inner.MaxAxisId();
    auto min_sah = FLT_MAX;

	// distribute the primitives into bins
	unsigned	bin[BVH_SPLIT_COUNT];
	BBox		bbox[BVH_SPLIT_COUNT];
	BBox		rbox[BVH_SPLIT_COUNT-1];
	memset( bin , 0 , sizeof( unsigned ) * BVH_SPLIT_COUNT );
    auto split_start = inner.m_Min[axis];
    auto split_delta = inner.Delta(axis) * BVH_INV_SPLIT_COUNT;
    if( split_delta == 0.0f )
        return FLT_MAX;
    auto inv_split_delta = 1.0f / split_delta;
    for(auto i = start ; i < end ; i++ ){
		int index = (int)((m_bvhpri[i].m_centroid[axis] - split_start) * inv_split_delta);
		index = std::min( index , (int)(BVH_SPLIT_COUNT - 1) );
		bin[index]++;
		bbox[index].Union( m_bvhpri[i].GetBBox() );
	}

	rbox[BVH_SPLIT_COUNT-2].Union( bbox[BVH_SPLIT_COUNT-1] );
	for( int i = BVH_SPLIT_COUNT-3; i >= 0 ; i-- )
		rbox[i] = Union( rbox[i+1] , bbox[i+1] );

    auto	left = bin[0];
    auto	lbox = bbox[0];
    auto    pos = split_delta + split_start ;
	for(auto i = 0 ; i < BVH_SPLIT_COUNT - 1 ; i++ ){
        auto sah_value = sah( left , primitive_num - left , lbox , rbox[i] , node->bbox );
		if( sah_value < min_sah ){
			min_sah = sah_value;
			splitPos = pos;
		}
		left += bin[i+1];
		lbox.Union( bbox[i+1] );
		pos += split_delta;
	}

	return min_sah;
}

float Bvh::sah( unsigned left , unsigned right , const BBox& lbox , const BBox& rbox , const BBox& box ){
	return (left * lbox.HalfSurfaceArea() + right * rbox.HalfSurfaceArea()) / box.HalfSurfaceArea();
}

void Bvh::makeLeaf( Bvh_Node* node , unsigned start , unsigned end ){
	node->pri_num = end - start;
	node->pri_offset = start;

    SORT_STATS(++sBvhLeafNodeCount);
    SORT_STATS(sBvhMaxPriCountInLeaf = std::max( sBvhMaxPriCountInLeaf , (StatsInt)node->pri_num) );
    SORT_STATS(sBvhPrimitiveCount += (StatsInt)node->pri_num);
}

bool Bvh::traverseNode( const Bvh_Node* node , const Ray& ray , Intersection* intersect , float fmin ) const{
	if( fmin < 0.0f )
		return false;

	if( intersect && intersect->t < fmin )
		return true;
	
    if( node->pri_num != 0 ){
        auto _start = node->pri_offset;
        auto _pri = node->pri_num;
        auto _end = _start + _pri;
        
        auto inter = false;
        for(auto i = _start ; i < _end ; i++ ){
            SORT_STATS(++sIntersectionTest);
            inter |= m_bvhpri[i].primitive->GetIntersect( ray , intersect );
            if( intersect == nullptr && inter )
                return true;
        }
        return inter;
	}

    auto left = node->left.get();
    auto right = node->right.get();

    auto fmin0 = Intersect( ray , left->bbox );
    auto fmin1 = Intersect( ray , right->bbox );

    auto inter = false;
	if( fmin1 > fmin0 ){
		inter |= traverseNode( left , ray , intersect , fmin0 );
		if( inter && intersect == nullptr ) return true;
		inter |= traverseNode( right , ray , intersect , fmin1 );
	}else{
		inter |= traverseNode( right , ray , intersect , fmin1 );
		if( inter && intersect == nullptr) return true;
		inter |= traverseNode( left , ray , intersect , fmin0 );
	}
    return intersect == nullptr ? inter : true;
}