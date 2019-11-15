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
        makeLeaf( node , start , end );
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

	int i = 0;
	if( to_split.size() + done_splitting.size() == 1 ){
		makeLeaf( node , start , end );
		return;
	}else{
		const auto populate_child = [&] ( Qbvh_Node* node , std::queue<std::pair<unsigned,unsigned>>& q ){
			while (!q.empty()) {
				const auto cur = q.front();
				q.pop();
				node->children[i++] = std::make_unique<Qbvh_Node>( cur.first , cur.second - cur.first );
			}
		};

		populate_child( node , to_split );
		populate_child( node , done_splitting );
	}

    // split children if needed.
    for( int j = 0 ; j < i ; ++j ){
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

    SORT_STATS(sQbvhNodeCount+=i);
}

void Qbvh::makeLeaf( Qbvh_Node* const node , unsigned start , unsigned end ){
    node->pri_cnt = end - start;
    node->pri_offset = start;
    node->leaf_node = true;

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
    SORT_PROFILE("Traverse Qbvh");
    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += intersect != nullptr);

    // pre-calculate some data
    ray.Prepare();

    const auto fmin = Intersect(ray, m_bbox);
    if (fmin < 0.0f)
        return false;

    if (traverseNode(m_root.get(), ray, intersect, fmin))
        return !intersect || intersect->primitive ;
    return false;
}

bool Qbvh::traverseNode( const Qbvh_Node* node , const Ray& ray , Intersection* intersect , float fmin ) const{
    if( fmin < 0.0f )
        return false;

    if( intersect && intersect->t < fmin )
        return true;

    if( node->leaf_node ){
        auto _start = node->pri_offset;
        auto _pri = node->pri_cnt;
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

#ifndef SSE_ENABLED
	int child_cnt = 0;
	float dist[QBVH_CHILD_CNT] = { FLT_MAX };
    for( ; child_cnt < QBVH_CHILD_CNT && node->children[child_cnt] ; ++child_cnt )
        dist[child_cnt] = Intersect( ray , node->bbox[child_cnt] );

    auto intersection_found = false;
    for( int i = 0 ; i < child_cnt ; ++i ){
        int k = 0;
        float minDist = FLT_MAX;
        for( int j = 0 ; j < child_cnt ; ++j ){
            if( dist[j] < minDist ){
                minDist = dist[j];
                k = j;
            }
        }

        dist[k] = FLT_MAX;
        intersection_found |= traverseNode( node->children[k].get() , ray , intersect , minDist );
        if( intersection_found && !intersect )
            return true;
    }
#else
	__m128 sse_f_max , sse_f_min;
	IntersectBBox4( ray , node->bbox , sse_f_min , sse_f_max );

	float f_max[4] , f_min[4];
	_mm_storeu_ps(f_max , sse_f_max);
	_mm_storeu_ps(f_min , sse_f_min);

	int child_cnt = 0;
	while( child_cnt < QBVH_CHILD_CNT && node->children[child_cnt] )
		++child_cnt;

	auto intersection_found = false;
    for( int i = 0 ; i < child_cnt ; ++i ){
        int k = -1;
        float minDist = FLT_MAX;
        for( int j = 0 ; j < child_cnt ; ++j ){
            if( f_min[j] < minDist && f_min[j] <= f_max[j] ){
                minDist = f_min[j];
                k = j;
            }
        }

        if( k == -1 )
            break;

        f_min[k] = FLT_MAX;
        intersection_found |= traverseNode( node->children[k].get() , ray , intersect , minDist );
        if( intersection_found && !intersect )
            return true;
    }
#endif
    return intersect == nullptr ? intersection_found : true;
}

void Qbvh::GetIntersect( const Ray& ray , BSSRDFIntersections& intersect , const StringID matID ) const{
    SORT_PROFILE("Traverse Qbvh");
    SORT_STATS(++sRayCount);

    // pre-calculate some data
    ray.Prepare();
    
    intersect.cnt = 0;
    intersect.maxt = FLT_MAX;

    const auto fmin = Intersect(ray, m_bbox);
    if( fmin < 0.0f )
        return;
    traverseNode(m_root.get(), ray, intersect, fmin, matID);
}

void Qbvh::traverseNode( const Qbvh_Node* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , const StringID matID ) const{
    sAssert( fmin >= 0.0f , SPATIAL_ACCELERATOR );

    if( intersect.maxt < fmin )
        return;

    if( node->leaf_node ){
        auto _start = node->pri_offset;
        auto _pri = node->pri_cnt;
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

#ifndef SSE_ENABLED
    int child_cnt = 0;
    float dist[QBVH_CHILD_CNT] = { FLT_MAX };

    for( ; child_cnt < QBVH_CHILD_CNT && node->children[child_cnt] ; ++child_cnt )
        dist[child_cnt] = Intersect( ray , node->bbox[child_cnt] );

    for( int i = 0 ; i < child_cnt ; ++i ){
        int k = 0;
        float minDist = FLT_MAX;
        for( int j = 0 ; j < child_cnt ; ++j ){
            if( dist[j] < minDist ){
                minDist = dist[j];
                k = j;
            }
        }

        dist[k] = FLT_MAX;
        if( minDist >= 0 )
            traverseNode( node->children[k].get() , ray , intersect , minDist , matID );
    }
#else
	__m128 sse_f_max , sse_f_min;
	IntersectBBox4( ray , node->bbox , sse_f_min , sse_f_max );

	float f_max[4] , f_min[4];
	_mm_storeu_ps(f_max , sse_f_max);
	_mm_storeu_ps(f_min , sse_f_min);

	int child_cnt = 0;
	while( child_cnt < QBVH_CHILD_CNT && node->children[child_cnt] )
		++child_cnt;

    for( int i = 0 ; i < child_cnt ; ++i ){
        int k = -1;
        float minDist = FLT_MAX;
        for( int j = 0 ; j < child_cnt ; ++j ){
            if( f_min[j] < minDist && f_min[j] <= f_max[j] ){
                minDist = f_min[j];
                k = j;
            }
        }

        if( k == -1 )
            break;

        f_min[k] = FLT_MAX;
        traverseNode( node->children[k].get() , ray , intersect , minDist , matID );
    }
#endif
}
