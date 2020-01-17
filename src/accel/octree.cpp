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

#include "octree.h"
#include "core/primitive.h"
#include "core/log.h"
#include "scatteringevent/scatteringevent.h"
#include "core/memory.h"

IMPLEMENT_RTTI(OcTree);

SORT_STATS_DEFINE_COUNTER(sOcTreeNodeCount)
SORT_STATS_DEFINE_COUNTER(sOcTreeLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sOcTreeDepth)
SORT_STATS_DEFINE_COUNTER(sOcTreeMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sOcTreePrimitiveCount)

SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "Intersection Test", sIntersectionTest);
SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "Node Count", sOcTreeNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "Leaf Node Count", sOcTreeLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "OcTree Depth", sOcTreeDepth);
SORT_STATS_COUNTER("Spatial-Structure(OcTree)", "Maximum Primitive in Leaf", sOcTreeMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(OcTree)", "Average Primitive Count in Leaf", sOcTreePrimitiveCount , sOcTreeLeafNodeCount);
SORT_STATS_AVG_COUNT("Spatial-Structure(OcTree)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

void OcTree::Build( const Scene& scene ){
    SORT_PROFILE("Build OcTree");

    m_primitives = scene.GetPrimitives();

    // handling empty mesh case
    if( m_primitives->size() == 0 )
        return ;

    // generate AABB
    computeBBox();

    // initialize a primitive container
    std::unique_ptr<NodePrimitiveContainer> container = std::make_unique<NodePrimitiveContainer>();
    for( auto& primitive : *m_primitives )
        container->primitives.push_back( primitive.get() );

    // create root node
    m_root = std::make_unique<OcTreeNode>();
    m_root->bb = m_bbox;

    // split OCTree node
    splitNode( m_root.get() , container.get() , 1u );

    m_isValid = true;

    SORT_STATS(++sOcTreeNodeCount);
}

void OcTree::splitNode( OcTreeNode* node , NodePrimitiveContainer* container , unsigned depth ){
    SORT_STATS( sOcTreeDepth = std::max( sOcTreeDepth , (StatsInt) depth ) );

    // make a leaf if there are not enough points
    if( container->primitives.size() <= (int)m_maxPriInLeaf || depth >= m_maxDepthInOcTree ){
        makeLeaf( node , container );
        return;
    }

    // container for child node
    std::unique_ptr<NodePrimitiveContainer> childcontainer[8];
    for (auto i = 0; i < 8; ++i) {
        node->child[i] = std::make_unique<OcTreeNode>();
        childcontainer[i] = std::make_unique<NodePrimitiveContainer>();
    }

    // get the center point of this tree node
    auto offset = 0;
    auto length = ( node->bb.m_Max - node->bb.m_Min ) * 0.5f;
    for(auto i = 0 ; i < 2; ++i ){
        for(auto j = 0 ; j < 2 ; ++j ){
            for(auto k = 0 ; k < 2 ; ++k ){
                // setup the lower left bottom point
                node->child[offset]->bb.m_Min = node->bb.m_Min + Vector( (float)k , (float)j , (float)i ) * length;
                node->child[offset]->bb.m_Max = node->child[offset]->bb.m_Min + length;
                ++offset;
            }
        }
    }

    // distribute primitives
    std::vector<const Primitive*>::const_iterator it = container->primitives.begin();
    while( it != container->primitives.end() ){
        for(auto i = 0 ; i < 8 ; ++i ){
            // check for intersection
            if( (*it)->GetIntersect( node->child[i]->bb ) )
                childcontainer[i]->primitives.push_back( *it );
        }
        ++it;
    }

    // There are cases where primitive lie along diagonal direction and it will be
    // extremely difficult, if not impossible, to separate them from different nodes.
    // In these very case, we need to stop immediately to avoid memory exploration.
    auto total_child_pri = 0;
    for(auto i = 0 ; i < 8 ; ++i )
        total_child_pri += (int)childcontainer[i]->primitives.size();
    if( total_child_pri > (int)(2 * container->primitives.size()) && depth > 8 ){
        // make leaf
        makeLeaf( node , container );

        // splitting plane information is no useful anymore.
        for (int i = 0; i < 8; ++i)
            node->child[i].reset();

        // no need to process any more
        return;
    }

    // split children node
    for(auto i = 0 ; i < 8 ; ++i )
        splitNode( node->child[i].get() , childcontainer[i].get() , depth + 1 );

    SORT_STATS(sOcTreeNodeCount+=8);
}

void OcTree::makeLeaf( OcTreeNode* node , NodePrimitiveContainer* container ){
    SORT_STATS(++sOcTreeLeafNodeCount);
    SORT_STATS(sOcTreeMaxPriCountInLeaf = std::max( sOcTreeMaxPriCountInLeaf , (StatsInt)container->primitives.size()) );
    SORT_STATS(sOcTreePrimitiveCount += (StatsInt)container->primitives.size());

    for( auto primitive : container->primitives )
        node->primitives.push_back( primitive );
}

bool OcTree::GetIntersect( const Ray& r , SurfaceInteraction& intersect ) const{
    SORT_PROFILE("Traverse OcTree");
    SORT_STATS(++sRayCount);

#ifdef ENABLE_TRANSPARENT_SHADOW
    SORT_STATS(sShadowRayCount += intersect.query_shadow);
#endif

    r.Prepare();

    float fmax;
    auto fmin = Intersect( r , m_bbox , &fmax );
    if( fmin < 0.0f )
        return false;

    return traverseOcTree( m_root.get() , r , &intersect , fmin , fmax );
}

#ifndef ENABLE_TRANSPARENT_SHADOW
bool OcTree::IsOccluded( const Ray& r ) const{
    SORT_PROFILE("Traverse OcTree");
    SORT_STATS(++sRayCount);
    SORT_STATS(++sShadowRayCount);

    r.Prepare();

    float fmax;
    auto fmin = Intersect( r , m_bbox , &fmax );
    if( fmin < 0.0f )
        return false;

    return traverseOcTree( m_root.get() , r , nullptr , fmin , fmax );
}
#endif

bool OcTree::traverseOcTree( const OcTreeNode* node , const Ray& ray , SurfaceInteraction* intersect , float fmin , float fmax ) const{
    constexpr auto   delta = 0.001f;
    auto found = false;

    // Early rejections
    if( fmin >= fmax )
        return false;
    if( intersect && intersect->t < fmin - delta )
        return true;

    // Iterate if there is primitives in the node. Since it is not allowed to store primitives in non-leaf node, there is no need to proceed.
    if( node->child[0] == nullptr ){
        for( auto primitive : node->primitives ){
            SORT_STATS(++sIntersectionTest);
            found |= primitive->GetIntersect( ray , intersect );

            // a quick branching out if a shadow ray is hit by an opaque object
            const auto is_shadow_ray_blocked = isShadowRay( intersect ) && found;
            if( is_shadow_ray_blocked ){
#ifdef ENABLE_TRANSPARENT_SHADOW
                sAssert( nullptr != intersect->primitive , SPATIAL_ACCELERATOR );
                sAssert( nullptr != intersect->primitive->GetMaterial() , SPATIAL_ACCELERATOR );
                if( !intersect->primitive->GetMaterial()->HasTransparency() ){
                    // setting primitive to be nullptr and return true at the same time is a special 'code' 
                    // that the above level logic will take advantage of.
                    intersect->primitive = nullptr;
                }
#endif
                return true;
            }
        }
        return found && ( intersect->t < ( fmax + delta ) && intersect->t > ( fmin - delta ) );
    }

    const auto contact = ray(fmin);
    const auto center = ( node->bb.m_Max + node->bb.m_Min ) * 0.5f;
    auto node_index = ( contact.x > center.x ) + ( contact.y > center.y ) * 2 + ( contact.z > center.z ) * 4;

    auto            _curt = fmin;
    int             _dir[3];
    float           _delta[3],_next[3];
    for(auto i = 0 ; i < 3 ; i++ ){
        _dir[i] = ( ray.m_Dir[i] > 0.0f ) ? 1 : -1;
        _delta[i] = ( ray.m_Dir[i] != 0.0f )?fabs( node->bb.Delta(i) / ray.m_Dir[i] ) * 0.5f : FLT_MAX;
    }
    for(auto i = 0 ; i < 3 ; i++ ){
        const auto target = node->child[node_index]->bb.m_Min[i] + ((_dir[i]+1)>>1) * node->bb.Delta(i) * 0.5f;
        _next[i] = ( ray.m_Dir[i] == 0.0f )?FLT_MAX:( target - ray.m_Ori[i] ) / ray.m_Dir[i];
    }

    // traverse the OcTree
    while( ( intersect && _curt < intersect->t ) || !intersect ){
        // get the axis along which the ray leaves the node fastest.
        auto nextAxis = (_next[0] <= _next[1]) ? 0 : 1;
        nextAxis = (_next[nextAxis] <= _next[2]) ? nextAxis : 2;

        // check if there is intersection in the current grid
        if( traverseOcTree( node->child[node_index].get() , ray , intersect , _curt , _next[nextAxis] ) )
            return true;

        // get to the next node based on distance
        node_index += ( 1 << nextAxis ) * _dir[nextAxis];

        // update next
        _curt = _next[nextAxis];
        _next[nextAxis] += _delta[nextAxis];

        // check for early rejection
        if( node_index < 0 || node_index > 7 )
            break;
        if( _curt > fmax + delta || _curt < fmin - delta )
            break;
    }

    return found;
}

void OcTree::GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID ) const{
    SORT_PROFILE("Traverse OcTree");
    SORT_STATS(++sRayCount);

    r.Prepare();

    intersect.cnt = 0;
    intersect.maxt = FLT_MAX;

    float fmax;
    auto fmin = Intersect( r , m_bbox , &fmax );
    if( fmin < 0.0f )
        return;

    traverseOcTree( m_root.get() , r , intersect , fmin , fmax , matID );
}

void OcTree::traverseOcTree( const OcTreeNode* node , const Ray& ray , BSSRDFIntersections& intersect , float fmin , float fmax , const StringID matID ) const{
    constexpr auto   delta = 0.001f;

    // early rejections
    if( fmin >= fmax )
        return;
    if( intersect.maxt < fmin )
        return;

    // iterate if there is primitives in the node. Since it is not allowed to store primitives in non-leaf node, there is no need to proceed.
    if( node->child[0] == nullptr ){
        SurfaceInteraction intersection;
        for( auto primitive : node->primitives ){
            if( matID != primitive->GetMaterial()->GetID() )
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

    const auto contact = ray(fmin);
    const auto center = ( node->bb.m_Max + node->bb.m_Min ) * 0.5f;
    auto node_index = ( contact.x > center.x ) + ( contact.y > center.y ) * 2 + ( contact.z > center.z ) * 4;

    auto            _curt = fmin;
    int             _dir[3];
    float           _delta[3],_next[3];
    for(auto i = 0 ; i < 3 ; i++ ){
        _dir[i] = ( ray.m_Dir[i] > 0.0f ) ? 1 : -1;
        _delta[i] = ( ray.m_Dir[i] != 0.0f )?fabs( node->bb.Delta(i) / ray.m_Dir[i] ) * 0.5f : FLT_MAX;
    }
    for(auto i = 0 ; i < 3 ; i++ ){
        const auto target = node->child[node_index]->bb.m_Min[i] + ((_dir[i]+1)>>1) * node->bb.Delta(i) * 0.5f;
        _next[i] = ( ray.m_Dir[i] == 0.0f )?FLT_MAX:( target - ray.m_Ori[i] ) / ray.m_Dir[i];
    }

    // traverse the OcTree
    while( _curt < intersect.maxt ){
        // get the axis along which the ray leaves the node fastest.
        auto nextAxis = (_next[0] <= _next[1]) ? 0 : 1;
        nextAxis = (_next[nextAxis] <= _next[2]) ? nextAxis : 2;

        // check if there is intersection in the current grid
        traverseOcTree( node->child[node_index].get() , ray , intersect , _curt , _next[nextAxis] , matID );

        // get to the next node based on distance
        node_index += ( 1 << nextAxis ) * _dir[nextAxis];

        // update next
        _curt = _next[nextAxis];
        _next[nextAxis] += _delta[nextAxis];

        // check for early rejection
        if( node_index < 0 || node_index > 7 )
            break;
        if( _curt > fmax + delta || _curt < fmin - delta )
            break;
    }
}