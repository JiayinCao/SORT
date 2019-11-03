/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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
#include "math/intersection.h"
#include "scatteringevent/scatteringevent.h"
#include "core/memory.h"

IMPLEMENT_RTTI(Bvh);

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

static const unsigned   BVH_SPLIT_COUNT         = 16;
static const float      BVH_INV_SPLIT_COUNT     = 0.0625f;

void Bvh::Build(const Scene& scene){
    SORT_PROFILE("Build Bvh");

    m_primitives = scene.GetPrimitives();
    m_bvhpri = std::make_unique<Bvh_Primitive[]>(m_primitives->size());

    // build bounding box
    computeBBox();

    // generate BVH primitives
    for (auto i = 0u; i < m_primitives->size(); ++i)
        m_bvhpri[i].SetPrimitive((*m_primitives)[i].get());

    // recursively split node
    m_root = std::make_unique<Bvh_Node>();
    splitNode( m_root.get() , 0u , (unsigned)m_primitives->size() , 1u );

    m_isValid = true;

    SORT_STATS(++sBvhNodeCount);
    SORT_STATS(sBvhLeafNodeCountCopy = sBvhLeafNodeCount);
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
    auto middle = std::partition( m_bvhpri.get() + start , m_bvhpri.get() + end, compare );
    auto mid = (unsigned)(middle - m_bvhpri.get());

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
    unsigned    bin[BVH_SPLIT_COUNT];
    BBox        bbox[BVH_SPLIT_COUNT];
    BBox        rbox[BVH_SPLIT_COUNT-1];
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

    auto    left = bin[0];
    auto    lbox = bbox[0];
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

bool Bvh::GetIntersect(const Ray& ray, Intersection* intersect) const{
    SORT_PROFILE("Traverse Bvh");
    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += intersect != nullptr);

    const auto fmin = Intersect(ray, m_bbox);
    if (fmin < 0.0f)
        return false;

    if (traverseNode(m_root.get(), ray, intersect, fmin))
        return !intersect || intersect->primitive ;
    return false;
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

        auto found = false;
        for(auto i = _start ; i < _end ; i++ ){
            SORT_STATS(++sIntersectionTest);
            found |= m_bvhpri[i].primitive->GetIntersect( ray , intersect );
            if( intersect == nullptr && found )
                return true;
        }
        return found;
    }

    const auto left = node->left.get();
    const auto right = node->right.get();

    const auto fmin0 = Intersect( ray , left->bbox );
    const auto fmin1 = Intersect( ray , right->bbox );

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

void Bvh::GetIntersect( const Ray& ray , BSSRDFIntersections& intersect , const StringID matID ) const{
    SORT_PROFILE("Traverse Bvh");
    SORT_STATS(++sRayCount);

    intersect.cnt = 0;
    intersect.maxt = FLT_MAX;

    const auto fmin = Intersect(ray, m_bbox);
    if( fmin < 0.0f )
        return;
    traverseNode(m_root.get(), ray, intersect, fmin, matID);
}

void Bvh::traverseNode( const Bvh_Node* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , const StringID matID ) const{
    sAssert( fmin >= 0.0f , SPATIAL_ACCELERATOR );

    if( intersect.maxt < fmin )
        return;

    if( 0 != node->pri_num ){
        auto _start = node->pri_offset;
        auto _pri = node->pri_num;
        auto _end = _start + _pri;

        Intersection intersection;
        for(auto i = _start ; i < _end ; i++ ){
            if( matID != m_bvhpri[i].primitive->GetMaterial()->GetID() )
                continue;
            SORT_STATS(++sIntersectionTest);
        
            intersection.Reset();
            const auto intersected = m_bvhpri[i].primitive->GetIntersect( ray , &intersection );
            if( intersected ){
                if( intersect.cnt < TOTAL_SSS_INTERSECTION_CNT ){
                    intersect.intersections[intersect.cnt] = SORT_MALLOC(BSSRDFIntersection)();
                    intersect.intersections[intersect.cnt++]->intersection = intersection;
                }else{
                    auto picked_i = -1;
                    auto t = 0.0f;
                    for( auto i = 0 ; i < TOTAL_SSS_INTERSECTION_CNT ; ++i ){
                        if( t < intersect.intersections[i]->intersection.t ){
                            t = intersect.intersections[i]->intersection.t;
                            picked_i = i;
                        }
                    }
                    if( picked_i >= 0 )
                        intersect.intersections[picked_i]->intersection = intersection;

                    intersect.maxt = 0.0f;
                    for( auto i = 0u ; i < intersect.cnt ; ++i )
                        intersect.maxt = std::max( intersect.maxt , intersect.intersections[i]->intersection.t );
                }
            }
        }

       return;
    }

    const auto left = node->left.get();
    const auto right = node->right.get();

    const auto fmin0 = Intersect( ray , left->bbox );
    const auto fmin1 = Intersect( ray , right->bbox );

    if( fmin1 > fmin0 ){
        if( fmin0 >= 0.0f )
            traverseNode( left , ray , intersect , fmin0 , matID );
        if( fmin1 >= 0.0f )
            traverseNode( right , ray , intersect , fmin1 , matID );
    }else{
        if( fmin1 >= 0.0f )
            traverseNode( right , ray , intersect , fmin1 , matID );
        if( fmin0 >= 0.0f )
            traverseNode( left , ray , intersect , fmin0 , matID );
    }
}
