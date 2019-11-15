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
#include "qbvh.h"
#include "core/memory.h"
#include "core/stats.h"
#include "scatteringevent/bssrdf/bssrdf.h"

#include "shape/simd_triangle.h"

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

static SORT_FORCEINLINE BBox calcBoundingBox(const Qbvh_Node* const node , const Bvh_Primitive* const primitives ) {
	BBox node_bbox;
	for (auto i = node->pri_offset; node && i < node->pri_offset + node->pri_cnt; i++)
		node_bbox.Union(primitives[i].GetBBox());
	return node_bbox;
}

void Qbvh::Build(const Scene& scene){
    SORT_PROFILE("Build Qbvh");

    m_primitives = scene.GetPrimitives();
    m_bvhpri = std::make_unique<Bvh_Primitive[]>(m_primitives->size());

    // build bounding box
    computeBBox();

    // generate BVH primitives
    for (auto i = 0u; i < m_primitives->size(); ++i)
        m_bvhpri[i].SetPrimitive((*m_primitives)[i].get());
    
    // recursively split node
    m_root = std::make_unique<Qbvh_Node>(0,(unsigned)m_primitives->size());
    splitNode( m_root.get() , m_bbox , 1u );

    // if the algorithm reaches here, it is a valid QBVH
    m_isValid = true;

    SORT_STATS(++sQbvhNodeCount);
    SORT_STATS(sQbvhLeafNodeCountCopy = sQbvhLeafNodeCount);
}

void Qbvh::splitNode( Qbvh_Node* const node , const BBox& node_bbox , unsigned depth ){
    SORT_STATS(sQbvhDepth = std::max( sQbvhDepth , (StatsInt)depth + 1 ) );

    const auto start    = node->pri_offset;
    const auto end      = start + node->pri_cnt;

    const auto primitive_num = end - start;
    if( primitive_num <= m_maxPriInLeaf || depth == m_maxNodeDepth ){
        makeLeaf( node , start , end , depth );
        return;
    }

	std::queue< std::pair<unsigned int, unsigned int> > to_split, done_splitting;
	to_split.push( std::make_pair( start , end ) );

	while( !to_split.empty() && to_split.size() + done_splitting.size() < (unsigned int)QBVH_CHILD_CNT ){
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
		const auto populate_child = [&] ( Qbvh_Node* node , std::queue<std::pair<unsigned,unsigned>>& q ){
			while (!q.empty()) {
				const auto cur = q.front();
				q.pop();
				node->children[node->child_cnt++] = std::make_unique<Qbvh_Node>( cur.first , cur.second - cur.first );
			}
		};

		populate_child( node , to_split );
		populate_child( node , done_splitting );
	}

    // split children if needed.
    for( int j = 0 ; j < node->child_cnt ; ++j ){
#ifndef SSE_ENABLED
        node->bbox[j] = calcBoundingBox( node->children[j].get() , m_bvhpri.get() );
		splitNode( node->children[j].get() , node->bbox[j] , depth + 1 );
#else
        const auto bbox = calcBoundingBox(node->children[j].get(), m_bvhpri.get());
		splitNode(node->children[j].get(), bbox, depth + 1);
#endif
    }

#ifdef SSE_ENABLED
	node->bbox = calcBoundingBox4( node->children[0].get() , node->children[1].get() , node->children[2].get() , node->children[3].get() );
#endif

    SORT_STATS(sQbvhNodeCount+=node->child_cnt);
}

void Qbvh::makeLeaf( Qbvh_Node* const node , unsigned start , unsigned end , unsigned depth ){
    node->pri_cnt = end - start;
    node->pri_offset = start;
    node->child_cnt = 0;

	m_depth = fmax( m_depth , depth );

    SORT_STATS(++sQbvhLeafNodeCount);
    SORT_STATS(sQbvhMaxPriCountInLeaf = std::max( sQbvhMaxPriCountInLeaf , (StatsInt)node->pri_cnt) );
    SORT_STATS(sQbvhPrimitiveCount += (StatsInt)node->pri_cnt);
}

#ifdef SSE_ENABLED

BBox4 Qbvh::calcBoundingBox4(const Qbvh_Node* const node0 , const Qbvh_Node* const node1 , const Qbvh_Node* const node2 , const Qbvh_Node* const node3 ) const {
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

bool Qbvh::GetIntersect( const Ray& ray , Intersection* intersect ) const{
	// std::stack is by no means an option here due to its overhead under the hood.
	static thread_local std::unique_ptr<std::pair<Qbvh_Node*, float>[]> bvh_stack = nullptr;
	if (UNLIKELY(nullptr == bvh_stack))
		bvh_stack = std::make_unique<std::pair<Qbvh_Node*, float>[]>(m_depth * QBVH_CHILD_CNT);

    SORT_PROFILE("Traverse Qbvh");
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

        // check if it is a leaf node, to be optimized by SSE/AVX
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

#ifndef SSE_ENABLED
        float f_min[QBVH_CHILD_CNT] = { FLT_MAX };
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
#else
        __m128 sse_f_max , sse_f_min;
        IntersectBBox4( ray , node->bbox , sse_f_min , sse_f_max );

        alignas(16) float f_max[4] , f_min[4];
        _mm_store_ps(f_max , sse_f_max);
        _mm_store_ps(f_min , sse_f_min);

        for( auto i = 0 ; i < node->child_cnt ; ++i ){
            auto k = -1;
            auto maxDist = -1.0f;
            for( int j = 0 ; j < node->child_cnt ; ++j ){
                if( f_min[j] > maxDist && f_min[j] <= f_max[j] ){
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

void Qbvh::GetIntersect( const Ray& ray , BSSRDFIntersections& intersect , const StringID matID ) const{
	// std::stack is by no means an option here due to its overhead under the hood.
	static thread_local std::unique_ptr<std::pair<Qbvh_Node*, float>[]> bvh_stack = nullptr;
	if ( UNLIKELY( nullptr == bvh_stack ) )
		bvh_stack = std::make_unique<std::pair<Qbvh_Node*, float>[]>(m_depth * QBVH_CHILD_CNT);

    SORT_PROFILE("Traverse Qbvh");
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

						intersect.maxt = 0.0f;
						for (auto i = 0u; i < intersect.cnt; ++i)
							intersect.maxt = std::max(intersect.maxt, intersect.intersections[i]->intersection.t);
					}
				}
			}

			continue;
		}

#ifndef SSE_ENABLED
		float f_min[QBVH_CHILD_CNT] = { FLT_MAX };
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
#else
		__m128 sse_f_max, sse_f_min;
		IntersectBBox4(ray, node->bbox, sse_f_min, sse_f_max);

		alignas(16) float f_max[4], f_min[4];
		_mm_store_ps(f_max, sse_f_max);
		_mm_store_ps(f_min, sse_f_min);

		for (auto i = 0; i < node->child_cnt; ++i) {
			auto k = -1;
			auto maxDist = -1.0f;
			for (auto j = 0; j < node->child_cnt; ++j) {
				if (f_min[j] > maxDist && f_min[j] <= f_max[j]) {
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