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

#include <queue>
#include "core/memory.h"
#include "core/stats.h"
#include "scatteringevent/bssrdf/bssrdf.h"
#include "simd/sse_triangle.h"

#ifdef QBVH_IMPLEMENTATION
IMPLEMENT_RTTI(Qbvh);

SORT_STATS_DEFINE_COUNTER(sQbvhNodeCount)
SORT_STATS_DEFINE_COUNTER(sQbvhLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sQbvhDepth)
SORT_STATS_DEFINE_COUNTER(sQbvhMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sQbvhPrimitiveCount)
SORT_STATS_DEFINE_COUNTER(sQbvhLeafNodeCountCopy)

SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Node Count", sQbvhNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Leaf Node Count", sQbvhLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "BVH Depth", sQbvhDepth);
SORT_STATS_COUNTER("Spatial-Structure(QBVH)", "Maximum Primitive in Leaf", sQbvhMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(QBVH)", "Average Primitive Count in Leaf", sQbvhPrimitiveCount , sQbvhLeafNodeCountCopy );
SORT_STATS_AVG_COUNT("Spatial-Structure(QBVH)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

#define sFbvhNodeCount			sQbvhNodeCount
#define sFbvhLeafNodeCount		sQbvhLeafNodeCount
#define sFbvhDepth				sQbvhDepth
#define sFbvhMaxPriCountInLeaf	sQbvhMaxPriCountInLeaf
#define sFbvhPrimitiveCount		sQbvhPrimitiveCount
#define sFbvhLeafNodeCountCopy	sQbvhLeafNodeCountCopy

#ifdef SSE_ENABLED
#define	SSE_QBVH_IMPLEMENTATION
#endif

#endif

#ifdef OBVH_IMPEMENTATION
IMPLEMENT_RTTI(Obvh);

SORT_STATS_DEFINE_COUNTER(sObvhNodeCount)
SORT_STATS_DEFINE_COUNTER(sObvhLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sObvhDepth)
SORT_STATS_DEFINE_COUNTER(sObvhMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sObvhPrimitiveCount)
SORT_STATS_DEFINE_COUNTER(sObvhLeafNodeCountCopy)

SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Node Count", sObvhNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Leaf Node Count", sObvhLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "BVH Depth", sObvhDepth);
SORT_STATS_COUNTER("Spatial-Structure(OBVH)", "Maximum Primitive in Leaf", sObvhMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(OBVH)", "Average Primitive Count in Leaf", sObvhPrimitiveCount , sObvhLeafNodeCountCopy );
SORT_STATS_AVG_COUNT("Spatial-Structure(OBVH)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

#define sFbvhNodeCount			sObvhNodeCount
#define sFbvhLeafNodeCount		sObvhLeafNodeCount
#define sFbvhDepth				sObvhDepth
#define sFbvhMaxPriCountInLeaf	sObvhMaxPriCountInLeaf
#define sFbvhPrimitiveCount		sObvhPrimitiveCount
#define sFbvhLeafNodeCountCopy	sObvhLeafNodeCountCopy

#ifdef AVX_ENABLED
#define AVX_OBVH_IMPLEMENTATION
#endif
#endif

static SORT_FORCEINLINE BBox calcBoundingBox(const Fbvh_Node* const node , const Bvh_Primitive* const primitives ) {
	BBox node_bbox;
	if (!node)
		return node_bbox;
	for (auto i = node->pri_offset; node && i < node->pri_offset + node->pri_cnt; i++)
		node_bbox.Union(primitives[i].GetBBox());
	return node_bbox;
}

void Fbvh::Build(const Scene& scene){
    SORT_PROFILE("Build Fbvh");

    m_primitives = scene.GetPrimitives();
    m_bvhpri = std::make_unique<Bvh_Primitive[]>(m_primitives->size());

    // build bounding box
    computeBBox();

    // generate BVH primitives
    for (auto i = 0u; i < m_primitives->size(); ++i)
        m_bvhpri[i].SetPrimitive((*m_primitives)[i].get());
    
    // recursively split node
    m_root = std::make_unique<Fbvh_Node>(0,(unsigned)m_primitives->size());
    splitNode( m_root.get() , m_bbox , 1u );

    // if the algorithm reaches here, it is a valid QBVH
    m_isValid = true;

    SORT_STATS(++sFbvhNodeCount);
    SORT_STATS(sFbvhLeafNodeCountCopy = sFbvhLeafNodeCount);
}

void Fbvh::splitNode( Fbvh_Node* const node , const BBox& node_bbox , unsigned depth ){
    SORT_STATS(sFbvhDepth = std::max( sFbvhDepth , (StatsInt)depth + 1 ) );

    const auto start    = node->pri_offset;
    const auto end      = start + node->pri_cnt;

    const auto primitive_num = end - start;
    if( primitive_num <= m_maxPriInLeaf || depth == m_maxNodeDepth ){
        makeLeaf( node , start , end , depth );
        return;
    }

	std::queue< std::pair<unsigned int, unsigned int> > to_split, done_splitting;
	to_split.push( std::make_pair( start , end ) );

	while( !to_split.empty() && to_split.size() + done_splitting.size() < (unsigned int)FBVH_CHILD_CNT ){
		auto cur_split = to_split.front();
		to_split.pop();

		const auto start	= cur_split.first;
		const auto end		= cur_split.second;
		const auto prim_cnt = end - start;

		unsigned    split_axis;
		float       split_pos;
		const auto sah = pickBestSplit(split_axis, split_pos, m_bvhpri.get(), node_bbox, start, end);
		if (sah >= prim_cnt)
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
				node->children[node->child_cnt++] = std::make_unique<Fbvh_Node>( cur.first , cur.second - cur.first );
			}
		};

		populate_child( node , to_split );
		populate_child( node , done_splitting );
	}

    // split children if needed.
    for( int j = 0 ; j < node->child_cnt ; ++j ){
#if defined(SSE_QBVH_IMPLEMENTATION)
		const auto bbox = calcBoundingBox(node->children[j].get(), m_bvhpri.get());
		splitNode(node->children[j].get(), bbox, depth + 1);
#elif defined( AVX_OBVH_IMPLEMENTATION )
		// to be implemented
#else
        node->bbox[j] = calcBoundingBox( node->children[j].get() , m_bvhpri.get() );
		splitNode( node->children[j].get() , node->bbox[j] , depth + 1 );
#endif
    }

#if defined(SSE_QBVH_IMPLEMENTATION)
	node->bbox = calcBoundingBox4( node->children[0].get() , node->children[1].get() , node->children[2].get() , node->children[3].get() );
#endif

#if defined(AVX_OBVH_IMPLEMENTATION)

#endif

    SORT_STATS(sFbvhNodeCount+=node->child_cnt);
}

void Fbvh::makeLeaf( Fbvh_Node* const node , unsigned start , unsigned end , unsigned depth ){
    node->pri_cnt = end - start;
    node->pri_offset = start;
    node->child_cnt = 0;

	m_depth = fmax( m_depth , depth );

#ifdef SSE_QBVH_IMPLEMENTATION
    Triangle4   tri4;
    Line4       line4;
    const auto _start = node->pri_offset;
    const auto _end = _start + node->pri_cnt;
    for(auto i = _start ; i < _end ; i++ ){
        const Primitive* primitive = m_bvhpri[i].primitive;
        const auto shape_type = primitive->GetShapeType();
        if( SHAPE_TRIANGLE == shape_type ){
            if( tri4.PushTriangle( primitive ) ){
                if( tri4.PackData() ){
					node->tri_list.push_back( tri4 );
					tri4.Reset();
				}
            }
        }else if( SHAPE_LINE == shape_type ){
            if( line4.PushLine( primitive ) ){
                if( line4.PackData() ){
                    node->line_list.push_back( line4 );
                    line4.Reset();
                }
            }
        }else{
            // line will also be specially treated in the future.
            node->other_list.push_back( primitive );
        }
    }
	if (tri4.PackData())
		node->tri_list.push_back(tri4);
    if (line4.PackData())
        node->line_list.push_back(line4);
#endif

#ifdef AVX_OBVH_IMPLEMENTATION
	// to be implemented
#endif

    SORT_STATS(++sFbvhLeafNodeCount);
    SORT_STATS(sFbvhMaxPriCountInLeaf = std::max( sFbvhMaxPriCountInLeaf , (StatsInt)node->pri_cnt) );
    SORT_STATS(sFbvhPrimitiveCount += (StatsInt)node->pri_cnt);
}

#ifdef SSE_QBVH_IMPLEMENTATION
BBox4 Fbvh::calcBoundingBox4(const Fbvh_Node* const node0 , const Fbvh_Node* const node1 , const Fbvh_Node* const node2 , const Fbvh_Node* const node3 ) const {
	BBox4 node_bbox;

	const BBox bb0 = calcBoundingBox( node0 , m_bvhpri.get() );
	const BBox bb1 = calcBoundingBox( node1 , m_bvhpri.get() );
	const BBox bb2 = calcBoundingBox( node2 , m_bvhpri.get() );
	const BBox bb3 = calcBoundingBox( node3 , m_bvhpri.get() );

	node_bbox.m_min_x = _mm_set_ps( bb3.m_Min.x , bb2.m_Min.x , bb1.m_Min.x , bb0.m_Min.x );
	node_bbox.m_min_y = _mm_set_ps( bb3.m_Min.y , bb2.m_Min.y , bb1.m_Min.y , bb0.m_Min.y );
	node_bbox.m_min_z = _mm_set_ps( bb3.m_Min.z , bb2.m_Min.z , bb1.m_Min.z , bb0.m_Min.z );
    
	node_bbox.m_max_x = _mm_set_ps( bb3.m_Max.x , bb2.m_Max.x , bb1.m_Max.x , bb0.m_Max.x );
	node_bbox.m_max_y = _mm_set_ps( bb3.m_Max.y , bb2.m_Max.y , bb1.m_Max.y , bb0.m_Max.y );
	node_bbox.m_max_z = _mm_set_ps( bb3.m_Max.z , bb2.m_Max.z , bb1.m_Max.z , bb0.m_Max.z );

	return node_bbox;
}
#endif

#ifdef AVX_OBVH_IMPLEMENTATION
BBox8 Fbvh::calcBoundingBox8( ... ) const{
	BBox8 node_bbox;
	return node_bbox;
}
#endif

bool Fbvh::GetIntersect( const Ray& ray , Intersection* intersect ) const{
	// std::stack is by no means an option here due to its overhead under the hood.
	static thread_local std::unique_ptr<std::pair<Fbvh_Node*, float>[]> bvh_stack = nullptr;
	if (UNLIKELY(nullptr == bvh_stack))
		bvh_stack = std::make_unique<std::pair<Fbvh_Node*, float>[]>(m_depth * FBVH_CHILD_CNT);

#ifdef QBVH_IMPLEMENTATION
	SORT_PROFILE("Traverse Qbvh");
#endif
#ifdef QBVH_IMPLEMENTATION
	SORT_PROFILE("Traverse Obvh");
#endif

    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += intersect != nullptr);

    // pre-calculate some data
    ray.Prepare();

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
        if( intersect && intersect->t < fmin )
            continue;

#if defined(SSE_QBVH_IMPLEMENTATION)
        // check if it is a leaf node
        if( 0 == node->child_cnt ){
            auto found = false;
            for( auto i = 0 ; i < node->tri_list.size() ; ++i ){
                found |= intersectTriangle4( ray , node->tri_list[i] , intersect );
                if( intersect == nullptr && found ){
                    SORT_STATS(sIntersectionTest+= i * 4);
                    return true;
                }
            }
            for( auto i = 0 ; i < node->line_list.size() ; ++i ){
                found |= intersectLine4( ray , node->line_list[i] , intersect );
                if( intersect == nullptr && found ){
                    SORT_STATS(sIntersectionTest+= ( i + node->tri_list.size() ) * 4 );
                    return true;
                }
            }
            if( UNLIKELY(!node->other_list.empty()) ){
                for( auto i = 0 ; i < node->other_list.size() ; ++i ){
                    found |= node->other_list[i]->GetIntersect( ray , intersect );
                    if( intersect == nullptr && found ){
                        SORT_STATS( sIntersectionTest += ( i + node->tri_list.size() + node->line_list.size() ) * 4 );
                        return true;
                    }
                }
            }
            SORT_STATS(sIntersectionTest+=node->pri_cnt);
            continue;
        }

		__m128 sse_f_min;
        auto mask = IntersectBBox4( ray , node->bbox , sse_f_min );

		auto m = _mm_movemask_ps(mask);
		if( 0 == m )
			continue;

		const int k0 = __bsf( m );
		const auto t0 = sse_data( sse_f_min , k0 );
		m &= m - 1;
		if( LIKELY( 0 == m ) ){
			sAssert( t0 >= 0.0f , SPATIAL_ACCELERATOR );
			bvh_stack[si++] = std::make_pair( node->children[k0].get() , t0 );
		}else{
			const int k1 = __bsf( m );
			m &= m - 1;

			if( LIKELY( 0 == m ) ){
				const auto t1 = sse_data( sse_f_min , k1 );
				sAssert( t1 >= 0.0f , SPATIAL_ACCELERATOR );

				if( t0 < t1 ){
					bvh_stack[si++] = std::make_pair(node->children[k1].get(), t1 );
					bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0 );
				}else{
					bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0);
					bvh_stack[si++] = std::make_pair(node->children[k1].get(), t1);
				}
			}else{
				// fall back to the worst case
				alignas(16) float f_min[4];
				_mm_store_ps(f_min, sse_f_min);

				for (auto i = 0; i < node->child_cnt; ++i) {
					auto k = -1;
					auto maxDist = 0.0f;
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
			}
		}
#elif defined(AVX_OBVH_IMPLEMENTATION)
		// to be implemented
#else
        // check if it is a leaf node
        if( 0 == node->child_cnt ){
            const auto _start = node->pri_offset;
            const auto _end = _start + node->pri_cnt;

            auto found = false;
            for(auto i = _start ; i < _end ; i++ ){
                found |= m_bvhpri[i].primitive->GetIntersect( ray , intersect );
                if( intersect == nullptr && found ){
                    SORT_STATS(sIntersectionTest+= i - _start + 1);
                    return true;
                }
            }
            SORT_STATS(sIntersectionTest+=node->pri_cnt);
            continue;
        }

        float f_min[FBVH_CHILD_CNT] = { FLT_MAX };
        for( int i = 0 ; i < node->child_cnt ; ++i )
            f_min[i] = Intersect( ray , node->bbox[i] );

        for( int i = 0 ; i < node->child_cnt ; ++i ){
            int k = -1;
            float maxDist = -1.0f;
            for( int j = 0 ; j < node->child_cnt ; ++j ){
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
    return intersect && intersect->primitive;
}

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

	// pre-calculate some data
	ray.Prepare();

	const auto fmin = Intersect(ray, m_bbox);
	if (fmin < 0.0f)
		return false;

	// stack index
	auto si = 0;
	bvh_stack[si++] = m_root.get();

	while (si > 0) {
		const auto node = bvh_stack[--si];

#ifdef SSE_QBVH_IMPLEMENTATION
		// check if it is a leaf node
		if (0 == node->child_cnt) {
			for (auto i = 0; i < node->tri_list.size(); ++i) {
				if (intersectTriangle4Fast(ray, node->tri_list[i])) {
					SORT_STATS(sIntersectionTest += i * 4);
					return true;
				}
			}
			for (auto i = 0; i < node->line_list.size(); ++i) {
				if (intersectLine4Fast(ray, node->line_list[i])) {
					SORT_STATS(sIntersectionTest += (i + node->tri_list.size()) * 4);
					return true;
				}
			}
			if (UNLIKELY(!node->other_list.empty())) {
				for (auto i = 0; i < node->other_list.size(); ++i) {
					if (node->other_list[i]->GetIntersect(ray, nullptr)) {
						SORT_STATS(sIntersectionTest += (i + node->tri_list.size() + node->line_list.size()) * 4);
						return true;
					}
				}
			}
			SORT_STATS(sIntersectionTest += node->pri_cnt);
			continue;
		}

		__m128 sse_f_min;
		auto mask = IntersectBBox4(ray, node->bbox, sse_f_min);

		auto m = _mm_movemask_ps(mask);
		if (0 == m)
			continue;

		const int k0 = __bsf(m);
		const auto t0 = sse_data(sse_f_min, k0);
		m &= m - 1;
		if (LIKELY(0 == m)) {
			sAssert(t0 >= 0.0f, SPATIAL_ACCELERATOR);
			bvh_stack[si++] = node->children[k0].get();
		}
		else {
			const int k1 = __bsf(m);
			const auto t1 = sse_data(sse_f_min, k1);
			m &= m - 1;

			sAssert(t1 >= 0.0f, SPATIAL_ACCELERATOR);

			if (LIKELY(0 == m)) {
				bvh_stack[si++] = node->children[k1].get();
				bvh_stack[si++] = node->children[k0].get();
			} else {
				const int k2 = __bsf(m);
				const auto t2 = sse_data(sse_f_min, k2);
				sAssert(t2 >= 0.0f, SPATIAL_ACCELERATOR);

				m &= m - 1;

				if( LIKELY(0==m) ){
					bvh_stack[si++] = node->children[k2].get();
					bvh_stack[si++] = node->children[k1].get();
					bvh_stack[si++] = node->children[k0].get();
				}else{
					const int k3 = __bsf(m);
					const auto t3 = sse_data(sse_f_min, k3);
					sAssert(t3 >= 0.0f, SPATIAL_ACCELERATOR);

					bvh_stack[si++] = node->children[k2].get();
					bvh_stack[si++] = node->children[k2].get();
					bvh_stack[si++] = node->children[k1].get();
					bvh_stack[si++] = node->children[k0].get();
				}
			}
		}
#elif defined(AVX_OBVH_IMPLEMENTATION)
		// to be implemented
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
		for (auto i = 0; i < node->child_cnt; ++i)
			f_min[i] = Intersect(ray, node->bbox[i]);

		for (auto i = 0; i < node->child_cnt; ++i)
			if( f_min[i] >= 0.0f )
				bvh_stack[si++] = node->children[i].get();
#endif
	}
	return false;
}

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

    // pre-calculate some data
    ray.Prepare();

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

#ifdef SSE_QBVH_IMPLEMENTATION
		if (0 == node->child_cnt) {
			// Note, only triangle shape support SSS here. This is the only big difference between SSE and non-SSE version implementation.
			// There are only two major primitives in SORT, line and triangle.
			// Line is usually used for hair, which has its own hair shader.
			// Triangle is the only major primitive that has SSS.
			for ( const auto& tri4 : node->tri_list )
				intersectTriangle4Multi(ray, tri4, matID, intersect);
			SORT_STATS(sIntersectionTest += node->pri_cnt);
			continue;
		}

		__m128 sse_f_min;
		auto mask = IntersectBBox4(ray, node->bbox, sse_f_min);

		auto m = _mm_movemask_ps(mask);
		if (0 == m)
			continue;

		const int k0 = __bsf(m);
		const auto t0 = sse_data(sse_f_min, k0);
		m &= m - 1;
		if (LIKELY(0 == m)) {
			sAssert(t0 >= 0.0f, SPATIAL_ACCELERATOR);
			bvh_stack[si++] = std::make_pair(node->children[k0].get(), t0);
		}
		else {
			const int k1 = __bsf(m);
			m &= m - 1;

			if (LIKELY(0 == m)) {
				const auto t1 = sse_data(sse_f_min, k1);
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
				alignas(16) float f_min[4];
				_mm_store_ps(f_min, sse_f_min);

				for (auto i = 0; i < node->child_cnt; ++i) {
					auto k = -1;
					auto maxDist = 0.0f;
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
			}
		}
#elif defined(AVX_OBVH_IMPLEMENTATION)
		// to be implemented
#else
		// check if it is a leaf node, to be optimized by SSE/AVX
		if (0 == node->child_cnt) {
			auto _start = node->pri_offset;
			auto _pri = node->pri_cnt;
			auto _end = _start + _pri;

			Intersection intersection;
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