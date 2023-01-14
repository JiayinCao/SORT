/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "triangle.h"
#include "entity/visual.h"

SORT_STATIC_FORCEINLINE Vector3f Permute( const Vector3f& v , int ax , int ay , int az ){
    return Vector3f( v[ax] , v[ay] , v[az] );
}

bool Triangle::GetIntersect( const Ray& r , SurfaceInteraction* intersect ) const{
    // get the memory
    // note : reference is not used here because it's not thread-safe
    auto& mem = m_meshVisual->m_memory;
    const auto id0 = m_index.m_id[0];
    const auto id1 = m_index.m_id[1];
    const auto id2 = m_index.m_id[2];

    const auto& mv0 = mem->m_vertices[id0];
    const auto& mv1 = mem->m_vertices[id1];
    const auto& mv2 = mem->m_vertices[id2];

    // get three vertexes
    const auto& op0 = mv0.m_position;
    const auto& op1 = mv1.m_position;
    const auto& op2 = mv2.m_position;

    auto p0 = op0;
    auto p1 = op1;
    auto p2 = op2;

    // transform the vertices from world space to ray coordinate
    // step 0 : translate the vertices with ray origin
    p0 -= r.m_Ori;
    p1 -= r.m_Ori;
    p2 -= r.m_Ori;

    // step 1 : pick the major axis to avoid dividing by zero in the sheering pass.
    //          by picking the major axis, we can also make sure we sheer as little as possible
    p0 = Permute( p0 , r.m_local_x , r.m_local_y , r.m_local_z );
    p1 = Permute( p1 , r.m_local_x , r.m_local_y , r.m_local_z );
    p2 = Permute( p2 , r.m_local_x , r.m_local_y , r.m_local_z );

    // step 2 : sheer the vertices so that the ray direction points to ( 0 , 1 , 0 )
    p0.x += r.m_scale_x * p0.y;
    p0.z += r.m_scale_z * p0.y;
    p1.x += r.m_scale_x * p1.y;
    p1.z += r.m_scale_z * p1.y;
    p2.x += r.m_scale_x * p2.y;
    p2.z += r.m_scale_z * p2.y;

    // compute the edge functions
    auto e0 = p1.x * p2.z - p1.z * p2.x;
    auto e1 = p2.x * p0.z - p2.z * p0.x;
    auto e2 = p0.x * p1.z - p0.z * p1.x;

    // fall back to double precision for better accuracy at some performance cost
    if( UNLIKELY( e0 == 0.0f || e1 == 0.0f || e2 == 0.0f ) ){
        e0 = (float)( (double)p1.x * (double)p2.z - (double)p1.z * (double)p2.x );
        e1 = (float)( (double)p2.x * (double)p0.z - (double)p2.z * (double)p0.x );
        e2 = (float)( (double)p0.x * (double)p1.z - (double)p0.z * (double)p1.x );
    }

    if( ( e0 < 0 || e1 < 0 || e2 < 0 ) && ( e0 > 0 || e1 > 0 || e2 > 0 ) )
        return false;
    const auto det = e0 + e1 + e2;
    if( det == .0f )
        return false;

    p0.y *= r.m_scale_y;
    p1.y *= r.m_scale_y;
    p2.y *= r.m_scale_y;
    const auto invDet = 1.0f / det;
    const auto t = ( e0 * p0.y + e1 * p1.y + e2 * p2.y ) * invDet;
    if( t <= r.m_fMin || t >= r.m_fMax )
        return false;
    if(IS_PTR_INVALID(intersect))
        return true;
    if( t > intersect->t || t <= 0.0f )
        return false;

    const auto u = e1 * invDet;
    const auto v = e2 * invDet;
    const auto w = 1 - u - v;

    // store the intersection
    intersect->intersect = r(t);

    intersect->gnormal = normalize(cross( ( op2 - op0 ) , ( op1 - op0 ) ));
    intersect->normal = ( w * mv0.m_normal + u * mv1.m_normal + v * mv2.m_normal).Normalize();
    intersect->tangent = ( w * mv0.m_tangent + u * mv1.m_tangent + v * mv2.m_tangent).Normalize();
    intersect->view = -r.m_Dir;

    const auto uv = w * mv0.m_texCoord + u * mv1.m_texCoord + v * mv2.m_texCoord;
    intersect->u = uv.x;
    intersect->v = uv.y;
    intersect->t = t;

    return true;
}

const BBox& Triangle::GetBBox() const{
    // if there is no bounding box , cache it
    if( !m_bbox ){
        m_bbox = std::make_unique<BBox>();

        const auto& mem = m_meshVisual->m_memory;
        const auto id0 = m_index.m_id[0];
        const auto id1 = m_index.m_id[1];
        const auto id2 = m_index.m_id[2];

        const auto& p0 = mem->m_vertices[id0].m_position;
        const auto& p1 = mem->m_vertices[id1].m_position;
        const auto& p2 = mem->m_vertices[id2].m_position;

        m_bbox->Union( p0 );
        m_bbox->Union( p1 );
        m_bbox->Union( p2 );
    }

    return *m_bbox;
}

float Triangle::SurfaceArea() const{
    const auto& mem = m_meshVisual->m_memory;
    const auto id0 = m_index.m_id[0];
    const auto id1 = m_index.m_id[1];
    const auto id2 = m_index.m_id[2];

    const auto& p0 = mem->m_vertices[id0].m_position;
    const auto& p1 = mem->m_vertices[id1].m_position;
    const auto& p2 = mem->m_vertices[id2].m_position;

    const auto e0 = p1 - p0 ;
    const auto e1 = p2 - p0 ;
    const auto t = cross( e0 , e1 );

    return t.Length() * 0.5f;
}

bool Triangle::GetIntersect(const BBox& box) const{
    // Project vertex along specific axis
    static const auto Project = [](const Point* points, int count , const Vector& axis, float& min, float& max){
        for( auto i = 0; i < count; ++i ){
            auto val = dot( axis , (Vector)points[i] );
            if (val < min) min = val;
            if (val > max) max = val;
        }
    };

    const auto& mem = m_meshVisual->m_memory;
    const auto id0 = m_index.m_id[0];
    const auto id1 = m_index.m_id[1];
    const auto id2 = m_index.m_id[2];

    const auto& mv0 = mem->m_vertices[id0];
    const auto& mv1 = mem->m_vertices[id1];
    const auto& mv2 = mem->m_vertices[id2];

    Point tri[3] = { mv0.m_position , mv1.m_position , mv2.m_position };

    float triMin , triMax;  // will initialize later
    auto boxMin = FLT_MAX, boxMax = -FLT_MAX;

    Vector  boxN[3] = { Vector( 1.0f , 0.0f , 0.0f ) ,
                        Vector( 0.0f , 1.0f , 0.0f ) ,
                        Vector( 0.0f , 0.0f , 1.0f )};

    // Case 1 : try separating axis perpendicular to box surface normal , 3 tests
    // along bounding box face normal directions first
    for( auto i = 0 ; i < 3 ; ++i ){
        triMin = FLT_MAX;
        triMax = -FLT_MAX;
        Project( tri , 3 , boxN[i] , triMin , triMax );
        if( triMin > box.m_Max[i] || triMax < box.m_Min[i] )
            return false;
    }

    // Case 2 : try triangle plane , 1 test
    // get triangle normal
    const auto triN = cross( tri[1] - tri[0] , tri[2] - tri[0] );
    //triN = triN * ( 1.0f / triN.Length() );   // no need to normalize it at all
    const auto triOffset = dot( triN , (Vector)tri[0] );
    Point bbp[8] = { box.m_Min ,
        Point( box.m_Min.x , box.m_Min.y , box.m_Max.z ) ,
        Point( box.m_Min.x , box.m_Max.y , box.m_Min.z ),
        Point( box.m_Min.x , box.m_Max.y , box.m_Max.z ),
        Point( box.m_Max.x , box.m_Min.y , box.m_Min.z ),
        Point( box.m_Max.x , box.m_Min.y , box.m_Max.z ),
        Point( box.m_Max.x , box.m_Max.y , box.m_Min.z ),
        box.m_Max };
    Project( bbp , 8 , triN , boxMin , boxMax );
    if( boxMax < triOffset || boxMin > triOffset )
        return false;

    // Case 3 : try cross product planes , 9 tests
    const Vector triangleEdges[3] = { tri[0] - tri[1] , tri[1] - tri[2] , tri[2] - tri[0] };
    for( auto i = 0 ; i < 3 ; ++i ){
        for( auto j = 0 ; j < 3 ; ++j ){
            triMin = boxMin = FLT_MAX;
            triMax = boxMax = -FLT_MAX;
            const auto new_axis = cross( triangleEdges[i] , boxN[j] );
            Project( bbp , 8 , new_axis , boxMin , boxMax );
            Project( tri , 3 , new_axis , triMin , triMax );

            if( boxMax < triMin || boxMin > triMax )
                return false;
        }
    }

    return true;
}
