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

#include <algorithm>
#include "kdtree.h"
#include "core/primitive.h"
#include "math/interaction.h"
#include "scatteringevent/scatteringevent.h"
#include "core/memory.h"

SORT_STATS_DEFINE_COUNTER(sKDTreeNodeCount)
SORT_STATS_DEFINE_COUNTER(sKDTreeLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sKDTreeDepth)
SORT_STATS_DEFINE_COUNTER(sKDTreeMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sKDTreePrimitiveCount)

SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Node Count", sKDTreeNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Leaf Node Count", sKDTreeLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "KDTree Depth", sKDTreeDepth);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Maximum Primitive in Leaf", sKDTreeMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(KDTree)", "Average Primitive Count in Leaf", sKDTreePrimitiveCount , sKDTreeLeafNodeCount );
SORT_STATS_AVG_COUNT("Spatial-Structure(KDTree)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

void KDTree::Build( const std::vector<const Primitive*>& primitives, const BBox& bbox){
    SORT_PROFILE("Build KdTree");

    m_primitives = &primitives;
    if (primitives.empty())
        return;

    // pre-malloc node and leaf primitive list memory
    auto tmp = std::make_unique<unsigned char[]>(m_primitives->size());

    m_bbox = bbox;

    // create the split candidates
    auto count = (unsigned int)m_primitives->size();
    Splits splits;
    for(auto i = 0 ; i < 3 ; i++ )
        splits.split[i] = std::make_unique<Split[]>(2*count);
    for(auto k = 0 ; k < 3 ; k++ ){
        for(auto i = 0u ; i < count ; i++ ){
            auto pri = (*m_primitives)[i];
            auto box = pri->GetBBox();
            splits.split[k][2*i] = Split(box.m_Min[k], Split_Type::Split_Start, i, pri);
            splits.split[k][2*i+1] = Split(box.m_Max[k], Split_Type::Split_End, i, pri);
        }
    }
    for(auto i = 0 ; i < 3 ; i++ )
        std::sort( splits.split[i].get() , splits.split[i].get() + 2 * count);

    // create root node
    m_root = std::make_unique<Kd_Node>(m_bbox);

    // build kd-tree
    splitNode( m_root.get() , splits , count , 1u , tmp.get() );

    SORT_STATS(++sKDTreeNodeCount);

    m_isValid = true;
}

void KDTree::splitNode( Kd_Node* node , Splits& splits , unsigned prinum , unsigned depth , unsigned char* tmp ){
    SORT_STATS(sKDTreeDepth = std::max(sKDTreeDepth, (StatsInt)depth));

    if( prinum < m_maxPriInLeaf || depth >= m_maxDepth ){
        makeLeaf( node , splits , prinum );
        return;
    }

    // ----------------------------------------------------------------------------------------
    // step 1
    // pick best split
    unsigned    split_offset;
    unsigned    split_Axis;
    auto sah = pickSplitting( splits , prinum , node->bbox , split_Axis , split_offset);
    if( sah >= prinum ){
        makeLeaf( node , splits , prinum );
        return;
    }
    node->flag = split_Axis;
    node->split = splits.split[split_Axis][split_offset].pos;

    // ----------------------------------------------------------------------------------------
    // step 2
    // distribute primitives
    const auto split_count = prinum * 2;
    auto _splits = splits.split[split_Axis].get();
    auto l_num = 0 , r_num = 0;
    for(auto i = 0u ; i < split_count; i++ )
        tmp[splits.split[split_Axis][i].id] = 0;
    for(auto i = 0u ; i < split_count; i++ )
    {
        if (i < split_offset) {
            if (_splits[i].type == Split_Type::Split_Start) {
                tmp[_splits[i].id] |= 1;
                ++l_num;
            }
        }
        else if (i > split_offset) {
            if (_splits[i].type == Split_Type::Split_End) {
                tmp[_splits[i].id] |= 2;
                ++r_num;
            }
        }
    }

    // ----------------------------------------------------------------------------------------
    // step 3
    // generate new events
    Splits l_splits;
    Splits r_splits;
    for(auto k = 0 ; k < 3 ; k++ ){
        l_splits.split[k] = std::make_unique<Split[]>(2*l_num);
        r_splits.split[k] = std::make_unique<Split[]>(2*r_num);
    }
    for(auto k = 0 ; k < 3 ; k++ ){
        auto l_offset = 0, r_offset = 0;
        for(auto i = 0u ; i < split_count ; i++ ){
            const Split& old = splits.split[k][i];
            auto id = old.id;
            if (tmp[id] & 0x01)
                l_splits.split[k][l_offset++] = old;
            if (tmp[id] & 0x02)
                r_splits.split[k][r_offset++] = old;
        }
        sAssert(l_offset == 2 * l_num, SPATIAL_ACCELERATOR);
        sAssert(r_offset == 2 * r_num, SPATIAL_ACCELERATOR);
    }

    auto left_box = node->bbox;
    left_box.m_Max[split_Axis] = node->split;
    node->leftChild = std::make_unique<Kd_Node>(left_box);
    splitNode( node->leftChild.get() , l_splits , l_num , depth + 1 , tmp );

    auto right_box = node->bbox;
    right_box.m_Min[split_Axis] = node->split;
    node->rightChild = std::make_unique<Kd_Node>(right_box);
    splitNode( node->rightChild.get() , r_splits , r_num , depth + 1 , tmp );

    SORT_STATS(sKDTreeNodeCount += 2);
}

float KDTree::sah( unsigned l , unsigned r , unsigned axis , float split , const BBox& box ){
    auto inv_sarea = 1.0f / box.HalfSurfaceArea();

    auto delta = box.m_Max - box.m_Min;
    delta[axis] = split - box.m_Min[axis];
    auto l_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;
    delta[axis] = box.m_Max[axis] - split;
    auto r_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;

    return ( l * l_sarea + r * r_sarea ) * inv_sarea;
}

float KDTree::pickSplitting( const Splits& splits , unsigned prinum , const BBox& box , unsigned& splitAxis , unsigned& split_offset ){
    auto min_sah = FLT_MAX;
    for(auto k = 0 ; k < 3 ; k++ ){
        auto n_l = 0 ;
        auto n_r = prinum ;
        auto split_count = prinum * 2;
        auto i = 0u;
        while( i < split_count ){
            if (splits.split[k][i].pos <= box.m_Min[k] ){
                ++i;
                continue;
            }
            if( splits.split[k][i].pos >= box.m_Max[k] )
                break;

            if (splits.split[k][i].type == Split_Type::Split_End)
                --n_r;

            // get the sah
            auto sahv = sah( n_l , n_r , k , splits.split[k][i].pos , box );
            if( sahv < min_sah ){
                min_sah = sahv;
                splitAxis = k;
                split_offset = i;
            }

            if (splits.split[k][i].type == Split_Type::Split_Start)
                ++n_l;

            ++i;
        }
    }

    return min_sah;
}

void KDTree::makeLeaf( Kd_Node* node , Splits& splits , unsigned prinum ){
    node->flag = 3;
    for(auto i = 0u ; i < prinum * 2; i++ ){
        if( splits.split[0][i].type == Split_Type::Split_Start ){
            const auto primitive = splits.split[0][i].primitive;
            if( primitive->GetIntersect( node->bbox ) )
                node->primitivelist.push_back(primitive);
        }
    }

    SORT_STATS(++sKDTreeLeafNodeCount);
    SORT_STATS(++sKDTreeNodeCount);
    SORT_STATS(sKDTreePrimitiveCount += prinum);
    SORT_STATS(sKDTreeMaxPriCountInLeaf = std::max(sKDTreeMaxPriCountInLeaf, (StatsInt)prinum));
}

bool KDTree::GetIntersect( RenderContext& rc, const Ray& r , SurfaceInteraction& intersect ) const{
    SORT_PROFILE("Traverse KD-Tree");
    SORT_STATS(++sRayCount);

#ifdef ENABLE_TRANSPARENT_SHADOW
    SORT_STATS(sShadowRayCount += intersect.query_shadow);
#endif

    r.Prepare();

    float fmax;
    auto fmin = Intersect( r , m_bbox , &fmax );
    if( fmin < 0.0f )
        return false;

    return traverse( m_root.get() , r , &intersect , fmin , fmax );
}

#ifndef ENABLE_TRANSPARENT_SHADOW
bool KDTree::IsOccluded( const Ray& r ) const{
    SORT_PROFILE("Traverse KD-Tree");
    SORT_STATS(++sRayCount);
    SORT_STATS(++sShadowRayCount);

    r.Prepare();

    float fmax;
    auto fmin = Intersect( r , m_bbox , &fmax );
    if( fmin < 0.0f )
        return false;

    return traverse( m_root.get() , r , nullptr , fmin , fmax );
}
#endif

bool KDTree::traverse( const Kd_Node* node , const Ray& ray , SurfaceInteraction* intersect , float fmin , float fmax ) const{
    static const auto       mask = 0x00000003u;
    static const auto       delta = 0.001f;

    if( fmin > fmax )
        return false;
    if( intersect && intersect->t < fmin - delta )
        return true;

    // it's a leaf node
    if( (node->flag & mask) == 3 ){
        auto inter = false;
        for( auto primitive : node->primitivelist ){
            SORT_STATS(++sIntersectionTest);
            inter |= primitive->GetIntersect( ray , intersect );
            if( isShadowRay( intersect ) && inter ){
#ifdef ENABLE_TRANSPARENT_SHADOW
                sAssert(IS_PTR_VALID( intersect->primitive ), SPATIAL_ACCELERATOR );
                sAssert(IS_PTR_VALID( intersect->primitive->GetMaterial() ), SPATIAL_ACCELERATOR );
                if( !intersect->primitive->GetMaterial()->HasTransparency() ){
                    // setting primitive to be nullptr and return true at the same time is a special 'code' 
                    // that the above level logic will take advantage of.
                    intersect->primitive = nullptr;
                }
#endif
                return true;
            }
        }
        return inter && ( intersect->t < ( fmax + delta ) && intersect->t > ( fmin - delta ) );
    }

    // get the intersection point between the ray and the splitting plane
    const auto split_axis = node->flag & mask;
    const auto dir = ray.m_Dir[split_axis];
    const auto t = (dir==0.0f) ? FLT_MAX : ( node->split - ray.m_Ori[split_axis] ) / dir;

    const auto* first = node->leftChild.get();
    const auto* second = node->rightChild.get();
    if( dir < 0.0f || (dir==0.0f&&ray.m_Ori[split_axis] > node->split) )
        std::swap(first, second);

    auto inter = false;
    if( t > fmin - delta ){
        inter = traverse( first , ray , intersect , fmin , std::min( fmax , t ) );
        if( isShadowRay(intersect) && inter )
            return true;
    }
    if( !inter && ( fmax + delta ) > t )
        return traverse( second , ray , intersect , std::max( t , fmin ) , fmax );
    return inter;
}

void KDTree::GetIntersect( const Ray& ray , BSSRDFIntersections& intersect , RenderContext& rc, const StringID matID ) const{
    SORT_PROFILE("Traverse KD-Tree");
    SORT_STATS(++sRayCount);
    
    ray.Prepare();

    intersect.cnt = 0;
    intersect.maxt = FLT_MAX;

    float fmax;
    auto fmin = Intersect( ray , m_bbox , &fmax );
    if( fmin < 0.0f )
        return;

    traverse( m_root.get() , ray , intersect , fmin , fmax , rc , matID );
}

void KDTree::traverse( const Kd_Node* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , float fmax , RenderContext& rc, const StringID matID ) const{
    static const auto       mask = 0x00000003u;
    static const auto       delta = 0.001f;

    if( fmin > fmax )
        return;

    if( intersect.maxt < fmin )
        return;

    // it's a leaf node
    if( (node->flag & mask) == 3 ){
        SurfaceInteraction intersection;
        
        for( auto primitive : node->primitivelist ){
            if( matID != primitive->GetMaterial()->GetUniqueID() )
                continue;
            
            // make sure the primitive is not checked before
            auto checked = false;
            for( auto i = 0u ; i < intersect.cnt ; ++i ){
                if( primitive == intersect.intersections[i]->intersection.primitive ){
                    checked = true;
                    break;
                }
            }
            if( checked )
                continue;

            SORT_STATS(++sIntersectionTest);
        
            intersection.Reset();
            const auto intersected = primitive->GetIntersect( ray , &intersection );
            if( intersected ){
                if( intersect.cnt < TOTAL_SSS_INTERSECTION_CNT ){
                    intersect.intersections[intersect.cnt] = SORT_MALLOC(rc.m_memory_arena,BSSRDFIntersection)();
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

    // get the intersection point between the ray and the splitting plane
    const auto split_axis = node->flag & mask;
    const auto dir = ray.m_Dir[split_axis];
    const auto t = (dir==0.0f) ? FLT_MAX : ( node->split - ray.m_Ori[split_axis] ) / dir;

    const auto* first = node->leftChild.get();
    const auto* second = node->rightChild.get();
    if( dir < 0.0f || ( dir==0.0f && ray.m_Ori[split_axis] > node->split ) )
        std::swap(first, second);

    if( t > fmin - delta )
        traverse( first , ray , intersect , fmin , std::min( fmax , t ) , rc , matID );
    if( ( fmax + delta ) > t )
        traverse( second , ray , intersect , std::max( t , fmin ) , fmax , rc , matID );
}

std::unique_ptr<Accelerator> KDTree::Clone() const {
    auto ret = std::make_unique<KDTree>();
    ret->m_maxDepth = m_maxDepth;
    ret->m_maxPriInLeaf = m_maxPriInLeaf;

    return ret;
}