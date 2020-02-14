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

#include <queue>
#include "core/memory.h"
#include "core/stats.h"
#include "scatteringevent/bssrdf/bssrdf.h"

SORT_STATIC_FORCEINLINE Fast_Bvh_Node_Ptr makeFastBvhNode( unsigned int start , unsigned int end ){
#ifdef SIMD_BVH_IMPLEMENTATION
    auto* address = malloc_aligned( sizeof(Fast_Bvh_Node) , SIMD_ALIGNMENT );
    auto* node = new (address) Fast_Bvh_Node( start , end );
    return std::move(Fast_Bvh_Node_Ptr(node));
#else
    return std::move( std::make_unique<Fast_Bvh_Node>( start , end ) );
#endif
}

template<class T>
SORT_STATIC_FORCEINLINE std::unique_ptr<T[]> makePrimitiveList( unsigned int cnt ){
#ifdef SIMD_BVH_IMPLEMENTATION
    auto* address = malloc_aligned( sizeof(T) * cnt , SIMD_ALIGNMENT );
    return std::move(std::unique_ptr<T[]>((T*)address));
#else
    return std::move(std::unique_ptr<T[]>(cnt));
#endif
}

#if defined(SIMD_SSE_IMPLEMENTATION) && defined(SIMD_AVX_IMPLEMENTATION)
static_assert(false, "More than one SIMD version is defined before including fast_bvh.hpp");
#endif

#ifdef QBVH_IMPLEMENTATION
IMPLEMENT_RTTI(Qbvh);

SORT_STATS_DEFINE_COUNTER(sQbvhNodeCount)
SORT_STATS_DEFINE_COUNTER(sQbvhLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sQbvhDepth)
SORT_STATS_DEFINE_COUNTER(sQbvhMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sQbvhPrimitiveCount)

SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Node Count", sQbvhNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Leaf Node Count", sQbvhLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "BVH Depth", sQbvhDepth);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Maximum Primitive in Leaf", sQbvhMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(QBVH)", "Average Primitive Count in Leaf", sQbvhPrimitiveCount , sQbvhLeafNodeCount );
SORT_STATS_AVG_COUNT("Spatial-Structure(QBVH)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

#define sFbvhNodeCount          sQbvhNodeCount
#define sFbvhLeafNodeCount      sQbvhLeafNodeCount
#define sFbvhDepth              sQbvhDepth
#define sFbvhMaxPriCountInLeaf  sQbvhMaxPriCountInLeaf
#define sFbvhPrimitiveCount     sQbvhPrimitiveCount

#endif

#ifdef OBVH_IMPEMENTATION
IMPLEMENT_RTTI(Obvh);

SORT_STATS_DEFINE_COUNTER(sObvhNodeCount)
SORT_STATS_DEFINE_COUNTER(sObvhLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sObvhDepth)
SORT_STATS_DEFINE_COUNTER(sObvhMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sObvhPrimitiveCount)

SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Node Count", sObvhNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Leaf Node Count", sObvhLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "BVH Depth", sObvhDepth);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Maximum Primitive in Leaf", sObvhMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(OBVH)", "Average Primitive Count in Leaf", sObvhPrimitiveCount , sObvhLeafNodeCount );
SORT_STATS_AVG_COUNT("Spatial-Structure(OBVH)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

#define sFbvhNodeCount          sObvhNodeCount
#define sFbvhLeafNodeCount      sObvhLeafNodeCount
#define sFbvhDepth              sObvhDepth
#define sFbvhMaxPriCountInLeaf  sObvhMaxPriCountInLeaf
#define sFbvhPrimitiveCount     sObvhPrimitiveCount

#endif

SORT_STATIC_FORCEINLINE BBox calcBoundingBox(const Fbvh_Node* const node , const Bvh_Primitive* const primitives ) {
    BBox node_bbox;
    if (!node)
        return node_bbox;
    for (auto i = node->pri_offset; node && i < node->pri_offset + node->pri_cnt; i++)
        node_bbox.Union(primitives[i].GetBBox());
    return node_bbox;
}

void Fbvh::Build(const std::vector<const Primitive*>& primitives){
    SORT_PROFILE("Build Fbvh");

    m_primitives = &primitives;
    m_bvhpri = std::make_unique<Bvh_Primitive[]>(m_primitives->size());

    // build bounding box
    computeBBox();

    // generate BVH primitives
    const auto primitive_cnt = m_primitives->size();
    for (auto i = 0u; i < primitive_cnt; ++i)
        m_bvhpri[i].SetPrimitive((*m_primitives)[i]);
    
    // recursively split node
    m_root = makeFastBvhNode( 0 , (unsigned)m_primitives->size() );
    splitNode( m_root.get() , m_bbox , 1u );

    // if the algorithm reaches here, it is a valid QBVH
    m_isValid = true;

    SORT_STATS(++sFbvhNodeCount);
    SORT_STATS(sFbvhPrimitiveCount += (StatsInt)primitive_cnt);
}

void Fbvh::splitNode( Fbvh_Node* const node , const BBox& node_bbox , unsigned depth ){
    SORT_STATS(sFbvhDepth = std::max( sFbvhDepth , (StatsInt)depth ) );

    const auto start    = node->pri_offset;
    const auto end      = start + node->pri_cnt;

    if( node->pri_cnt <= m_maxPriInLeaf || depth == m_maxNodeDepth ){
        makeLeaf( node , start , end , depth );
        return;
    }

    std::queue< std::pair<unsigned int, unsigned int> > to_split, done_splitting;
    to_split.push( std::make_pair( start , end ) );

    while( !to_split.empty() && to_split.size() + done_splitting.size() < (unsigned int)FBVH_CHILD_CNT ){
        auto cur_split = to_split.front();
        to_split.pop();

        const auto start    = cur_split.first;
        const auto end      = cur_split.second;
        const auto prim_cnt = end - start;

        unsigned    split_axis;
        float       split_pos;
        const auto sah = pickBestSplit(split_axis, split_pos, m_bvhpri.get(), node_bbox, start, end);
        if (sah >= prim_cnt || prim_cnt <= m_maxPriInLeaf )
            done_splitting.push( std::make_pair( start , end ) );
        else{
            const auto compare = [split_pos, split_axis](const Bvh_Primitive& pri) {return pri.m_centroid[split_axis] < split_pos; };
            const auto middle = std::partition(m_bvhpri.get() + start, m_bvhpri.get() + end, compare);
            const auto mid = (unsigned)(middle - m_bvhpri.get());

            if (mid == start || mid == end){
                done_splitting.push( std::make_pair( start , end ) );
            }else{
                to_split.push( std::make_pair( start , mid ) );
                to_split.push( std::make_pair( mid , end ) );
            }
        }
    }

    if( to_split.size() + done_splitting.size() == 1 ){
        makeLeaf( node , start , end , depth );
        return;
    }else{
        const auto populate_child = [&] ( Fbvh_Node* node , std::queue<std::pair<unsigned,unsigned>>& q ){
            while (!q.empty()) {
                const auto cur = q.front();
                q.pop();
                node->children[node->child_cnt++] = makeFastBvhNode( cur.first , cur.second - cur.first );
            }
        };

        populate_child( node , to_split );
        populate_child( node , done_splitting );
    }

    // split children if needed.
    for( auto j = 0u ; j < node->child_cnt ; ++j ){
#ifdef SIMD_BVH_IMPLEMENTATION
        const auto bbox = calcBoundingBox(node->children[j].get(), m_bvhpri.get());
        splitNode(node->children[j].get(), bbox, depth + 1);
#else
        node->bbox[j] = calcBoundingBox( node->children[j].get() , m_bvhpri.get() );
        splitNode( node->children[j].get() , node->bbox[j] , depth + 1 );
#endif
    }

#ifdef SIMD_BVH_IMPLEMENTATION
    node->bbox = calcBoundingBoxSIMD( node->children );
#endif

    SORT_STATS(sFbvhNodeCount+=node->child_cnt);
}

void Fbvh::makeLeaf( Fbvh_Node* const node , unsigned start , unsigned end , unsigned depth ){
    node->pri_cnt = end - start;
    node->pri_offset = start;
    node->child_cnt = 0;

    m_depth = fmax( m_depth , depth );

#ifdef SIMD_BVH_IMPLEMENTATION
    Simd_Triangle   sind_tri;
    Simd_Line       simd_line;
    std::vector<Simd_Triangle>  tri_list;
    std::vector<Simd_Line>      line_list;
    const auto _start = node->pri_offset;
    const auto _end = _start + node->pri_cnt;
    for(auto i = _start ; i < _end ; i++ ){
        const Primitive* primitive = m_bvhpri[i].primitive;
        const auto shape_type = primitive->GetShapeType();
        if( SHAPE_TRIANGLE == shape_type ){
            if( sind_tri.PushTriangle( primitive ) ){
                if( sind_tri.PackData() ){
                    tri_list.push_back( sind_tri );
                    sind_tri.Reset();
                }
            }
        }else if( SHAPE_LINE == shape_type ){
            if( simd_line.PushLine( primitive ) ){
                if( simd_line.PackData() ){
                    line_list.push_back( simd_line );
                    simd_line.Reset();
                }
            }
        }else{
            // line will also be specially treated in the future.
            node->other_list.push_back( primitive );
        }
    }
    if (sind_tri.PackData())
        tri_list.push_back(sind_tri);
    if (simd_line.PackData())
        line_list.push_back(simd_line);
    
    if( tri_list.size() ){
        node->tri_list = makePrimitiveList<Simd_Triangle>( (unsigned int)tri_list.size() );
        node->tri_cnt = (unsigned int)tri_list.size();
        for( auto i = 0u ; i < tri_list.size() ; ++i )
            node->tri_list[i] = tri_list[i];
    }
    if( line_list.size() ){
        node->line_list = makePrimitiveList<Simd_Line>( (unsigned int)line_list.size() );
        node->line_cnt = (unsigned int)line_list.size();
        for( auto i = 0u ; i < line_list.size() ; ++i )
            node->line_list[i] = line_list[i];
    }
#endif

    SORT_STATS(++sFbvhLeafNodeCount);
    SORT_STATS(sFbvhMaxPriCountInLeaf = std::max( sFbvhMaxPriCountInLeaf , (StatsInt)node->pri_cnt) );
}

#ifdef SIMD_BVH_IMPLEMENTATION
Simd_BBox Fbvh::calcBoundingBoxSIMD(const Fast_Bvh_Node_Ptr* children) const {
    Simd_BBox node_bbox;

    float   min_x[SIMD_CHANNEL] , min_y[SIMD_CHANNEL] , min_z[SIMD_CHANNEL];
    float   max_x[SIMD_CHANNEL] , max_y[SIMD_CHANNEL] , max_z[SIMD_CHANNEL];
    bool    bb_valid[SIMD_CHANNEL] = { false };
    for( auto i = 0 ; i < SIMD_CHANNEL ; ++i ){
        const auto bb = calcBoundingBox( children[i].get() , m_bvhpri.get() );
        min_x[i] = bb.m_Min.x;
        min_y[i] = bb.m_Min.y;
        min_z[i] = bb.m_Min.z;
        max_x[i] = bb.m_Max.x;
        max_y[i] = bb.m_Max.y;
        max_z[i] = bb.m_Max.z;

        bb_valid[i] = ( nullptr != children[i].get() );
    }

    node_bbox.m_min_x = simd_set_ps( min_x );
    node_bbox.m_min_y = simd_set_ps( min_y );
    node_bbox.m_min_z = simd_set_ps( min_z );
    
    node_bbox.m_max_x = simd_set_ps( max_x );
    node_bbox.m_max_y = simd_set_ps( max_y );
    node_bbox.m_max_z = simd_set_ps( max_z );

    node_bbox.m_mask = simd_set_mask( bb_valid );

    return node_bbox;
}
#endif

bool Fbvh::GetIntersect( const Ray& ray , SurfaceInteraction& intersect ) const{
    // std::stack is by no means an option here due to its overhead under the hood.
    static thread_local std::unique_ptr<std::pair<Fbvh_Node*, float>[]> bvh_stack = nullptr;
    if (UNLIKELY(nullptr == bvh_stack))
        bvh_stack = std::make_unique<std::pair<Fbvh_Node*, float>[]>(m_depth * FBVH_CHILD_CNT);

#ifdef QBVH_IMPLEMENTATION
    SORT_PROFILE("Traverse Qbvh");
#endif
#ifdef OBVH_IMPLEMENTATION
    SORT_PROFILE("Traverse Obvh");
#endif

    SORT_STATS(++sRayCount);

#ifdef ENABLE_TRANSPARENT_SHADOW
    SORT_STATS(sShadowRayCount += intersect.query_shadow);
#endif

    ray.Prepare();

#ifdef SIMD_BVH_IMPLEMENTATION
    Simd_Ray_Data   simd_ray;
    resolveRayData( ray , simd_ray );
#endif

    const auto fmin = Intersect(ray, m_bbox);
    if (fmin < 0.0f)
        return false;

    // stack index
    auto si = 0;
    bvh_stack[si++] = std::make_pair( m_root.get() , fmin );

    while( si > 0 ){
        const auto top = bvh_stack[--si];

        const auto node = top.first;
        const auto fmin = top.second;
        if( intersect.t < fmin )
            continue;

#ifdef SIMD_BVH_IMPLEMENTATION
        // check if it is a leaf node
        if( 0 == node->child_cnt ){
            for( auto i = 0u ; i < node->tri_cnt ; ++i ){
                const auto blocked = intersectTriangle_SIMD( ray , simd_ray , node->tri_list[i] , &intersect );

#ifdef ENABLE_TRANSPARENT_SHADOW
                // A quick branching out for shadow ray if there is no semi-transparent shadow
                // There is still possibility for false positives to survive this branch since only the nearest among four/eight possible intersections
                // will be tested here. If the nearest intersection happens to have transparency while not the others, it won't branch out, leading to
                // some potential defficiency. However, testing every single intersection in all possible intersections among all SIMD channels also 
                // comes at a cost and given the chance of mixing transparent primitive and non-transparent primitives in one BVH node is not fairly high, 
                // it makes sense to just check the nearest one. It should work pretty well for fully opaque scene.
                // With C++ 17 compile time if, this branch can totally be resolved during compilation, which may further reduce a bit of overhead, which
                // might not be very obvious. there could be ways to achieve it in C++ 11. Since it won't boost the performance, I will keep it this way
                // until I have C++ 17 updated.
                if( intersect.query_shadow && blocked ){
                    sAssert( nullptr != intersect.primitive , SPATIAL_ACCELERATOR );
                    sAssert( nullptr != intersect.primitive->GetMaterial() , SPATIAL_ACCELERATOR );
                    if( !intersect.primitive->GetMaterial()->HasTransparency() ){
                        SORT_STATS(sIntersectionTest += ( i + 1 ) * 4);

                        // setting primitive to be nullptr and return true at the same time is a special 'code' 
                        // that the above level logic will take advantage of.
                        intersect.primitive = nullptr;
                        return true;
                    }
                }
#endif
            }
            for( auto i = 0u ; i < node->line_cnt ; ++i ){
                const auto blocked = intersectLine_SIMD( ray , simd_ray , node->line_list[i] , &intersect );

#ifdef ENABLE_TRANSPARENT_SHADOW
                if( intersect.query_shadow && blocked ){
                    SORT_STATS(sIntersectionTest += (i + 1 + node->tri_cnt) * 4);
                    if( LIKELY(!intersect.primitive->GetMaterial()->HasTransparency()) ){
                        SORT_STATS(sIntersectionTest += i + 1 + ( node->tri_cnt ) * 4);
                        intersect.primitive = nullptr;
                    }
                    return true;
                }
#endif
            }
            if( UNLIKELY(!node->other_list.empty()) ){
                for( auto i = 0u ; i < node->other_list.size() ; ++i ){
                    const auto blocked = node->other_list[i]->GetIntersect( ray , &intersect );

#ifdef ENABLE_TRANSPARENT_SHADOW
                    if( intersect.query_shadow && blocked ){
                        sAssert( nullptr != intersect.primitive , SPATIAL_ACCELERATOR );
                        sAssert( nullptr != intersect.primitive->GetMaterial() , SPATIAL_ACCELERATOR );
                        if( !intersect.primitive->GetMaterial()->HasTransparency() ){
                            SORT_STATS(sIntersectionTest += i + 1 + ( node->tri_cnt + node->line_cnt ) * 4);
                            intersect.primitive = nullptr;
                            return true;
                        }
                    }
#endif
                }
            }
            SORT_STATS(sIntersectionTest+=node->pri_cnt);
            continue;
        }

        simd_data sse_f_min;
        auto m = IntersectBBox_SIMD( ray , simd_ray , node->bbox , sse_f_min );
        if( 0 == m )
            continue;

        const int k0 = __bsf( m );
        const auto t0 = sse_f_min[k0];
        m &= m - 1;
        if( LIKELY( 0 == m ) ){
            sAssert( t0 >= 0.0f , SPATIAL_ACCELERATOR );
            bvh_stack[si++] = std::make_pair( node->children[k0].get() , t0 );
        }else{
            const int k1 = __bsf( m );
            m &= m - 1;

            if( LIKELY( 0 == m ) ){
                const auto t1 = sse_f_min[k1];
                sAssert( t1 >= 0.0f , SPATIAL_ACCELERATOR );

                if( t0 < t1 ){
                    bvh_stack[si++] = std::make_pair(node->children[k1].get(), t1 );
                    bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0 );
                }else{
                    bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0);
                    bvh_stack[si++] = std::make_pair(node->children[k1].get(), t1);
                }
            }else{
                for (auto i = 0u; i < node->child_cnt; ++i) {
                    auto k = -1;
                    auto maxDist = -1.0f;
                    for (auto j = 0u; j < node->child_cnt; ++j) {
                        if (sse_f_min[j] > maxDist) {
                            maxDist = sse_f_min[j];
                            k = j;
                        }
                    }

                    if (k == -1)
                        break;

                    sse_f_min[k] = -1.0f;
                    bvh_stack[si++] = std::make_pair(node->children[k].get(), maxDist);
                }
            }
        }
#else
        // check if it is a leaf node
        if( 0 == node->child_cnt ){
            const auto _start = node->pri_offset;
            const auto _end = _start + node->pri_cnt;

            for(auto i = _start ; i < _end ; i++ ){
                const auto blocked = m_bvhpri[i].primitive->GetIntersect( ray , &intersect );

#ifdef ENABLE_TRANSPARENT_SHADOW
                if( intersect.query_shadow && blocked ){
                    sAssert( nullptr != intersect.primitive , SPATIAL_ACCELERATOR );
                    sAssert( nullptr != intersect.primitive->GetMaterial() , SPATIAL_ACCELERATOR );
                    if( !intersect.primitive->GetMaterial()->HasTransparency() ){
                        SORT_STATS(sIntersectionTest += i - _start + 1);
                        intersect.primitive = nullptr;
                        return true;
                    }
                }
#endif
            }
            SORT_STATS(sIntersectionTest+=node->pri_cnt);
            continue;
        }

        float f_min[FBVH_CHILD_CNT] = { FLT_MAX };
        for( auto i = 0u ; i < node->child_cnt ; ++i )
            f_min[i] = Intersect( ray , node->bbox[i] );

        for( auto i = 0u ; i < node->child_cnt ; ++i ){
            auto k = -1;
            auto maxDist = -1.0f;
            for( auto j = 0u ; j < node->child_cnt ; ++j ){
                if( f_min[j] > maxDist ){
                    maxDist = f_min[j];
                    k = j;
                }
            }

            if( k == -1 )
                break;

            f_min[k] = -1.0f;
            bvh_stack[si++] = std::make_pair( node->children[k].get() , maxDist );
        }
#endif
    }
    return intersect.primitive;
}

#ifndef ENABLE_TRANSPARENT_SHADOW
bool  Fbvh::IsOccluded(const Ray& ray) const{
    // std::stack is by no means an option here due to its overhead under the hood.
    using Fbvh_Node_Ptr = Fbvh_Node*;
    static thread_local std::unique_ptr<Fbvh_Node_Ptr[]> bvh_stack = nullptr;
    if (UNLIKELY(nullptr == bvh_stack))
        bvh_stack = std::make_unique<Fbvh_Node_Ptr[]>(m_depth * FBVH_CHILD_CNT);

#ifdef QBVH_IMPLEMENTATION
    SORT_PROFILE("Traverse Qbvh");
#endif
#ifdef QBVH_IMPLEMENTATION
    SORT_PROFILE("Traverse Obvh");
#endif

    SORT_STATS(++sRayCount);
    SORT_STATS(++sShadowRayCount);

    ray.Prepare();
#ifdef SIMD_BVH_IMPLEMENTATION
    Simd_Ray_Data   simd_ray;
    resolveRayData( ray , simd_ray );
#endif

    const auto fmin = Intersect(ray, m_bbox);
    if (fmin < 0.0f)
        return false;

    // stack index
    auto si = 0;
    bvh_stack[si++] = m_root.get();

    while (si > 0) {
        const auto node = bvh_stack[--si];

#ifdef SIMD_BVH_IMPLEMENTATION
        // check if it is a leaf node
        if (0 == node->child_cnt) {
            for (auto i = 0u; i < node->tri_cnt; ++i) {
                if (intersectTriangleFast_SIMD(ray, simd_ray , node->tri_list[i])) {
                    SORT_STATS(sIntersectionTest += ( i + 1 ) * 4);
                    return true;
                }
            }
            for (auto i = 0u; i < node->line_cnt; ++i) {
                if (intersectLineFast_SIMD(ray, simd_ray , node->line_list[i])) {
                    SORT_STATS(sIntersectionTest += (i + 1 + node->tri_cnt) * 4);
                    return true;
                }
            }
            if (UNLIKELY(!node->other_list.empty())) {
                for (auto i = 0u; i < node->other_list.size(); ++i) {
                    if (node->other_list[i]->GetIntersect(ray, nullptr)) {
                        SORT_STATS(sIntersectionTest += i + 1 + ( node->tri_cnt + node->line_cnt ) * 4);
                        return true;
                    }
                }
            }
            SORT_STATS(sIntersectionTest += node->pri_cnt);
            continue;
        }

        simd_data sse_f_min;
        auto m = IntersectBBox_SIMD(ray, simd_ray, node->bbox, sse_f_min);
        if (0 == m)
            continue;

        const int k0 = __bsf(m);
        m &= m - 1;
        if (LIKELY(0 == m)) {
            sAssert(sse_f_min[k0] >= 0.0f, SPATIAL_ACCELERATOR);
            bvh_stack[si++] = node->children[k0].get();
        }
        else {
            const int k1 = __bsf(m);
            m &= m - 1;

            sAssert(sse_f_min[k1] >= 0.0f, SPATIAL_ACCELERATOR);

            if (LIKELY(0 == m)) {
                bvh_stack[si++] = node->children[k1].get();
                bvh_stack[si++] = node->children[k0].get();
            } else {
                const int k2 = __bsf(m);
                sAssert(sse_f_min[k2] >= 0.0f, SPATIAL_ACCELERATOR);

                m &= m - 1;

                if( LIKELY(0==m) ){
                    bvh_stack[si++] = node->children[k2].get();
                    bvh_stack[si++] = node->children[k1].get();
                    bvh_stack[si++] = node->children[k0].get();
                }else{
#if defined(SIMD_AVX_IMPLEMENTATION)
                    for (auto i = 0u; i < node->child_cnt; ++i) {
                        auto k = -1;
                        auto maxDist = -1.0f;
                        for (auto j = 0u; j < node->child_cnt; ++j) {
                            if (sse_f_min[j] > maxDist) {
                                maxDist = sse_f_min[j];
                                k = j;
                            }
                        }

                        if (k == -1)
                            break;

                        sse_f_min[k] = -1.0f;
                        bvh_stack[si++] = node->children[k].get();
                    }
#endif
#if defined(SIMD_SSE_IMPLEMENTATION)
                    const int k3 = __bsf(m);
                    sAssert(sse_f_min[k3] >= 0.0f, SPATIAL_ACCELERATOR);

                    bvh_stack[si++] = node->children[k3].get();
                    bvh_stack[si++] = node->children[k2].get();
                    bvh_stack[si++] = node->children[k1].get();
                    bvh_stack[si++] = node->children[k0].get();
#endif
                }
            }
        }
#else
        // check if it is a leaf node
        if (0 == node->child_cnt) {
            const auto _start = node->pri_offset;
            const auto _end = _start + node->pri_cnt;

            for (auto i = _start; i < _end; i++) {
                if (m_bvhpri[i].primitive->GetIntersect(ray, nullptr)) {
                    SORT_STATS(sIntersectionTest += i - _start + 1);
                    return true;
                }
            }
            SORT_STATS(sIntersectionTest += node->pri_cnt);
            continue;
        }

        float f_min[FBVH_CHILD_CNT] = { FLT_MAX };
        for (auto i = 0u; i < node->child_cnt; ++i)
            f_min[i] = Intersect(ray, node->bbox[i]);

        for (auto i = 0u; i < node->child_cnt; ++i)
            if( f_min[i] >= 0.0f )
                bvh_stack[si++] = node->children[i].get();
#endif
    }
    return false;
}
#endif

void Fbvh::GetIntersect( const Ray& ray , BSSRDFIntersections& intersect , const StringID matID ) const{
    // std::stack is by no means an option here due to its overhead under the hood.
    static thread_local std::unique_ptr<std::pair<Fbvh_Node*, float>[]> bvh_stack = nullptr;
    if ( UNLIKELY( nullptr == bvh_stack ) )
        bvh_stack = std::make_unique<std::pair<Fbvh_Node*, float>[]>(m_depth * FBVH_CHILD_CNT);

#ifdef QBVH_IMPLEMENTATION
    SORT_PROFILE("Traverse Qbvh");
#endif
#ifdef QBVH_IMPLEMENTATION
    SORT_PROFILE("Traverse Obvh");
#endif

    SORT_STATS(++sRayCount);

    ray.Prepare();
#ifdef SIMD_BVH_IMPLEMENTATION
    Simd_Ray_Data   simd_ray;
    resolveRayData( ray , simd_ray );
#endif

    intersect.cnt = 0;
    intersect.maxt = FLT_MAX;

    const auto fmin = Intersect(ray, m_bbox);
    if (fmin < 0.0f)
        return;

    // stack index
    auto si = 0;
    bvh_stack[si++] = std::make_pair(m_root.get(), fmin);

    while (si > 0) {
        const auto top = bvh_stack[--si];

        const auto node = top.first;
        const auto fmin = top.second;
        if (intersect.maxt < fmin)
            continue;

#ifdef SIMD_BVH_IMPLEMENTATION
        if (0 == node->child_cnt) {
            // Note, only triangle shape support SSS here. This is the only big difference between AVX and non-AVX version implementation.
            // There are only two major primitives in SORT, line and triangle.
            // Line is usually used for hair, which has its own hair shader.
            // Triangle is the only major primitive that has SSS.
            for ( auto i = 0u ; i < node->tri_cnt ; ++i )
                intersectTriangleMulti_SIMD(ray, simd_ray, node->tri_list[i] , matID, intersect);
            SORT_STATS(sIntersectionTest += node->tri_cnt);
            continue;
        }

        simd_data sse_f_min;
        auto m = IntersectBBox_SIMD(ray, simd_ray, node->bbox, sse_f_min);
        if (0 == m)
            continue;

        const int k0 = __bsf(m);
        const auto t0 = sse_f_min[k0];
        m &= m - 1;
        if (LIKELY(0 == m)) {
            sAssert(t0 >= 0.0f, SPATIAL_ACCELERATOR);
            bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0);
        }
        else {
            const int k1 = __bsf(m);
            m &= m - 1;

            if (LIKELY(0 == m)) {
                const auto t1 = sse_f_min[k1];
                sAssert(t1 >= 0.0f, SPATIAL_ACCELERATOR);

                if (t0 < t1) {
                    bvh_stack[si++] = std::make_pair(node->children[k1].get(), t1);
                    bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0);
                }
                else {
                    bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0);
                    bvh_stack[si++] = std::make_pair(node->children[k1].get(), t1);
                }
            }
            else {
                // fall back to the worst case
                for (auto i = 0u; i < node->child_cnt; ++i) {
                    auto k = -1;
                    auto maxDist = -1.0f;
                    for (auto j = 0u; j < node->child_cnt; ++j) {
                        if (sse_f_min[j] > maxDist) {
                            maxDist = sse_f_min[j];
                            k = j;
                        }
                    }

                    if (k == -1)
                        break;

                    sse_f_min[k] = -1.0f;
                    bvh_stack[si++] = std::make_pair(node->children[k].get(), maxDist);
                }
            }
        }
#else
        // check if it is a leaf node, to be optimized by SSE/AVX
        if (0 == node->child_cnt) {
            auto _start = node->pri_offset;
            auto _pri = node->pri_cnt;
            auto _end = _start + _pri;

            SurfaceInteraction intersection;
            for (auto i = _start; i < _end; i++) {
                if (matID != m_bvhpri[i].primitive->GetMaterial()->GetID())
                    continue;

                SORT_STATS(++sIntersectionTest);

                intersection.Reset();
                const auto intersected = m_bvhpri[i].primitive->GetIntersect(ray, &intersection);
                if (intersected) {
                    if (intersect.cnt < TOTAL_SSS_INTERSECTION_CNT) {
                        intersect.intersections[intersect.cnt] = SORT_MALLOC(BSSRDFIntersection)();
                        intersect.intersections[intersect.cnt++]->intersection = intersection;
                    }
                    else {
                        auto picked_i = -1;
                        auto t = 0.0f;
                        for (auto i = 0; i < TOTAL_SSS_INTERSECTION_CNT; ++i) {
                            if (t < intersect.intersections[i]->intersection.t) {
                                t = intersect.intersections[i]->intersection.t;
                                picked_i = i;
                            }
                        }
                        if (picked_i >= 0)
                            intersect.intersections[picked_i]->intersection = intersection;

                        intersect.ResolveMaxDepth();
                    }
                }
            }

            continue;
        }

        float f_min[FBVH_CHILD_CNT] = { FLT_MAX };
        for (int i = 0; i < node->child_cnt; ++i)
            f_min[i] = Intersect(ray, node->bbox[i]);

        for (int i = 0; i < node->child_cnt; ++i) {
            int k = -1;
            float maxDist = -1.0f;
            for (int j = 0; j < node->child_cnt; ++j) {
                if (f_min[j] > maxDist) {
                    maxDist = f_min[j];
                    k = j;
                }
            }

            if (k == -1)
                break;

            f_min[k] = -1.0f;
            bvh_stack[si++] = std::make_pair(node->children[k].get(), maxDist);
        }
#endif
    }
}
