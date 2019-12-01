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

#include "unigrid.h"
#include "core/primitive.h"
#include "math/intersection.h"
#include "core/log.h"
#include "core/sassert.h"
#include "core/profile.h"
#include "scatteringevent/scatteringevent.h"
#include "core/memory.h"

IMPLEMENT_RTTI(UniGrid);

SORT_STATS_DEFINE_COUNTER(sUGGridCount)
SORT_STATS_DEFINE_COUNTER(sUniformGridX)
SORT_STATS_DEFINE_COUNTER(sUniformGridY)
SORT_STATS_DEFINE_COUNTER(sUniformGridZ)

SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Grid Count", sUGGridCount);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Dimension X", sUniformGridX);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Dimension Y", sUniformGridY);
SORT_STATS_COUNTER("Spatial-Structure(UniformGrid)", "Dimension Z", sUniformGridZ);
SORT_STATS_AVG_COUNT("Spatial-Structure(UniformGrid)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

void UniGrid::Build( const Scene& scene ){
    SORT_PROFILE("Build Uniform Grid");

    m_primitives = scene.GetPrimitives();

    if( nullptr == m_primitives || m_primitives->empty() ){
        slog( WARNING , SPATIAL_ACCELERATOR , "There is no primitive in uniform grid." );
        return;
    }

    // find the bounding box first
    computeBBox();

    // get the maximum extent id and distance
    auto id = m_bbox.MaxAxisId();
    auto delta = m_bbox.m_Max - m_bbox.m_Min;
    auto extent = delta[id];

    // get the total number of primitives
    auto count = (unsigned)m_primitives->size();

    // grid per distance
    auto gridPerDistance = 3 * powf( (float)count , 0.333f ) / extent ;

    // the grid size
    for(auto i = 0 ; i < 3 ; i++ ){
        m_voxelNum[i] = (unsigned)(std::min( 256.0f , gridPerDistance * delta[i] ));
        m_voxelInvExtent[i] = m_voxelNum[i] / delta[i];
        m_voxelExtent[i] = 1.0f / m_voxelInvExtent[i];
    }

    m_voxelCount = m_voxelNum[0] * m_voxelNum[1] * m_voxelNum[2];

    // allocate the memory
    m_voxels.resize( m_voxelCount );

    // distribute the primitives
    for( auto& primitive : *m_primitives ){
        unsigned maxGridId[3];
        unsigned minGridId[3];
        for(auto i = 0 ; i < 3 ; i++ ){
            minGridId[i] = point2VoxelId(primitive->GetBBox().m_Min , i );
            maxGridId[i] = point2VoxelId(primitive->GetBBox().m_Max , i );
        }

        for(auto i = minGridId[2] ; i <= maxGridId[2] ; i++ )
            for(auto j = minGridId[1] ; j <= maxGridId[1] ; j++ )
                for(auto k = minGridId[0] ; k <= maxGridId[0] ; k++ ){
                    BBox bb;
                    bb.m_Min = m_bbox.m_Min + Vector( (float)k , (float)j , (float)i ) * m_voxelExtent;
                    bb.m_Max = bb.m_Min + m_voxelExtent;

                    // only add the primitives if it is actually intersected
                    if( primitive->GetIntersect( bb ) )
                        m_voxels[offset( k , j , i )].push_back(primitive.get());
                }
    }

    m_isValid = true;

    SORT_STATS(sUniformGridX = m_voxelNum[0]);
    SORT_STATS(sUniformGridY = m_voxelNum[1]);
    SORT_STATS(sUniformGridZ = m_voxelNum[2]);
    SORT_STATS(sUGGridCount = m_voxelCount);
}

unsigned UniGrid::point2VoxelId( const Point& p , unsigned axis ) const{
    return std::min( m_voxelNum[axis] - 1 , (unsigned)( ( p[axis] - m_bbox.m_Min[axis] ) * m_voxelInvExtent[axis] ) );
}

// get the id offset
unsigned UniGrid::offset( unsigned x , unsigned y , unsigned z ) const{
    return z * m_voxelNum[1] * m_voxelNum[0] + y * m_voxelNum[0] + x;
}

bool UniGrid::GetIntersect( const Ray& r , Intersection* intersect ) const{
    SORT_PROFILE("Traverse Uniform Grid");
    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += intersect == nullptr);

    r.Prepare();

    static const auto voxelId2Point = [&]( int id[3] ){
        Point p;
        p.x = m_bbox.m_Min.x + id[0] * m_voxelExtent[0];
        p.y = m_bbox.m_Min.y + id[1] * m_voxelExtent[1];
        p.z = m_bbox.m_Min.z + id[2] * m_voxelExtent[2];
        return p;
    };

    // get the intersect point
    float maxt;
    auto cur_t = Intersect( r , m_bbox , &maxt );
    if( cur_t < 0.0f )
        return false;
    if( intersect )
        intersect->t = std::min( intersect->t , maxt );

    int     curGrid[3] , dir[3];
    float   delta[3] , next[3];
    for(auto i = 0 ; i < 3 ; i++ ){
        curGrid[i] = point2VoxelId( r(cur_t) , i );
        dir[i] = ( r.m_Dir[i] > 0.0f ) ? 1 : -1;
        if( r.m_Dir[i] != 0.0f )
            delta[i] = fabs( m_voxelExtent[i] / r.m_Dir[i] );
        else
            delta[i] = FLT_MAX;
    }
    Point gridCorner = voxelId2Point( curGrid );
    for(auto i = 0 ; i < 3 ; i++ ){
        // get the next t
        auto target = gridCorner[i] + ((dir[i]+1)>>1) * m_voxelExtent[i];
        next[i] = ( target - r.m_Ori[i] ) / r.m_Dir[i];
        if( r.m_Dir[i] == 0.0f )
            next[i] = FLT_MAX;
    }

    // traverse the uniform grid
    const unsigned idArray[] = { 0 , 0 , 1 , 0 , 2 , 2 , 1 , 0  };// [0] and [7] is impossible
    while( ( intersect && cur_t < intersect->t ) || ( intersect == 0 ) ){
        // current voxel id
        auto voxelId = offset( curGrid[0] , curGrid[1] , curGrid[2] );

        // get the next t
        auto nextAxis = (next[0] <= next[1])+((unsigned)(next[1] <= next[2]))*2+((unsigned)(next[2] <= next[0]))*4;
        nextAxis = idArray[nextAxis];

        // check if there is intersection in the current grid
        if( traverse( r , intersect , voxelId , next[nextAxis] ) )
            return true;

        // get to the next voxel
        curGrid[nextAxis] += dir[nextAxis];

        if( curGrid[nextAxis] < 0 || (unsigned)curGrid[nextAxis] >= m_voxelNum[nextAxis] )
            return intersect && intersect->t < maxt && intersect->primitive;

        // update next
        cur_t = next[nextAxis];
        next[nextAxis] += delta[nextAxis];
    }

    return ( intersect && intersect->t < maxt && ( intersect->primitive != nullptr ));
}

bool UniGrid::traverse( const Ray& r , Intersection* intersect , unsigned voxelId , float nextT ) const{
    sAssertMsg( voxelId < m_voxelCount , SPATIAL_ACCELERATOR , "Invalid voxel id." );

    auto inter = false;
    for( auto voxel : m_voxels[voxelId] ){
        SORT_STATS(++sIntersectionTest);
        // get intersection
        inter |= voxel->GetIntersect( r , intersect );
        if( !intersect && inter )
            return true;
    }

    return inter && ( intersect->t < nextT + 0.00001f );
}

void UniGrid::GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID ) const{
    SORT_PROFILE("Traverse Uniform Grid");
    SORT_STATS(++sRayCount);

    r.Prepare();

    intersect.cnt = 0;
    intersect.maxt = FLT_MAX;

    static const auto voxelId2Point = [&]( int id[3] ){
        Point p;
        p.x = m_bbox.m_Min.x + id[0] * m_voxelExtent[0];
        p.y = m_bbox.m_Min.y + id[1] * m_voxelExtent[1];
        p.z = m_bbox.m_Min.z + id[2] * m_voxelExtent[2];
        return p;
    };

    // get the intersect point
    float maxt;
    auto cur_t = Intersect( r , m_bbox , &maxt );
    if( cur_t < 0.0f )
        return;

    int     curGrid[3] , dir[3];
    float   delta[3] , next[3];
    for(auto i = 0 ; i < 3 ; i++ ){
        curGrid[i] = point2VoxelId( r(cur_t) , i );
        dir[i] = ( r.m_Dir[i] > 0.0f ) ? 1 : -1;
        if( r.m_Dir[i] != 0.0f )
            delta[i] = fabs( m_voxelExtent[i] / r.m_Dir[i] );
        else
            delta[i] = FLT_MAX;
    }
    Point gridCorner = voxelId2Point( curGrid );
    for(auto i = 0 ; i < 3 ; i++ ){
        // get the next t
        auto target = gridCorner[i] + ((dir[i]+1)>>1) * m_voxelExtent[i];
        next[i] = ( target - r.m_Ori[i] ) / r.m_Dir[i];
        if( r.m_Dir[i] == 0.0f )
            next[i] = FLT_MAX;
    }

    // traverse the uniform grid
    const unsigned idArray[] = { 0 , 0 , 1 , 0 , 2 , 2 , 1 , 0  };// [0] and [7] is impossible
    while( cur_t < intersect.maxt ){
        // current voxel id
        auto voxelId = offset( curGrid[0] , curGrid[1] , curGrid[2] );

        // get the next t
        auto nextAxis = (next[0] <= next[1])+((unsigned)(next[1] <= next[2]))*2+((unsigned)(next[2] <= next[0]))*4;
        nextAxis = idArray[nextAxis];

        // check if there is intersection in the current grid
        traverse( r , intersect , voxelId , next[nextAxis] , matID );

        // get to the next voxel
        curGrid[nextAxis] += dir[nextAxis];

        if( curGrid[nextAxis] < 0 || (unsigned)curGrid[nextAxis] >= m_voxelNum[nextAxis] )
            return;

        // update next
        cur_t = next[nextAxis];
        next[nextAxis] += delta[nextAxis];
    }
}

void UniGrid::traverse( const Ray& ray , BSSRDFIntersections& intersect , unsigned voxelId , float nextT , const StringID matID ) const{
    sAssertMsg( voxelId < m_voxelCount , SPATIAL_ACCELERATOR , "Invalid voxel id." );

    Intersection intersection;
    for( auto primitive : m_voxels[voxelId] ){
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
}