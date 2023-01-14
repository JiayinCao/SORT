/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <numeric>
#include "visual.h"
#include "material/matmanager.h"
#include "core/scene.h"
#include "accel/embree_util.h"
#include "accel/embree.h"

void MeshVisual::Serialize( IStreamBase& stream ){
    m_memory = std::make_unique<Mesh>();
    m_memory->Serialize(stream);

    for (const auto& mi : m_memory->m_indices){
        m_triangles.push_back( std::make_unique<Triangle>( this , mi ) );
        m_primitives.push_back(std::make_unique<Primitive>(m_memory.get(), mi.m_mat, m_triangles.back().get()));
    }
}

void MeshVisual::ApplyTransform( const Transform& transform ){
    m_memory->ApplyTransform( transform );
    m_memory->GenUV();
    m_memory->GenSmoothTagent();
}

void HairVisual::Serialize( IStreamBase& stream ){
    auto hair_cnt = 0u;
    auto width_tip = 0.0f , width_bottom = 0.0f;
    stream >> hair_cnt;
    stream >> width_tip >> width_bottom;
    auto mat_id = -1;
    stream >> mat_id;
    
    for( auto i = 0u ; i < hair_cnt ; ++i ){
        auto hair_step = 0u;
        stream >> hair_step;

        if (UNLIKELY(0u == hair_step))
            continue;

        // There is no guarrantee that the line segements will be the same length.
        // It is necessary to evaluate the total length of the hair before pushing them into the list to get correct UV and width data.
        std::vector<Point>  point_cache;
        std::vector<float>  len_cache(hair_step);
        for (auto j = 0u; j <= hair_step; ++j) {
            Point curP;
            stream >> curP;
            point_cache.push_back(curP);
        }
        for (auto j = 0u; j < point_cache.size() - 1; ++j)
            len_cache[j] += distance(point_cache[j], point_cache[j + 1]);

        // this means that the data is ill-defined, it shouldn't happen at all.
        const auto total_length = std::accumulate(len_cache.begin(), len_cache.end(), 0.0f);
        if (UNLIKELY(total_length <= 0.0f))
            continue;

        auto prev_v = 0.0f;
        auto prev_w = width_bottom;
        auto cur_len = 0.0f;
        for (auto j = 1u; j <= hair_step; ++j) {
            cur_len += len_cache[j-1];

            const auto& prevP = point_cache[j - 1];
            const auto& curP  = point_cache[j];

            const auto t = cur_len / total_length;
            const auto cur_w = slerp(width_bottom, width_tip, t);
            const auto cur_v = slerp(0.0f, 1.0f, t);

            m_lines.push_back(std::make_unique<Line>(prevP, curP, prev_v, cur_v, prev_w, cur_w, mat_id));

            auto mat = MatManager::GetSingleton().GetMaterial(m_lines.back()->GetMaterialId());
            m_primitives.push_back(std::make_unique<Primitive>(nullptr, mat, m_lines.back().get()));

            prev_w = cur_w;
            prev_v = cur_v;
        }
    }
}

void HairVisual::ApplyTransform( const Transform& transform ){
    for( auto& line : m_lines )
        line->SetTransform( transform );
}

SinglePrimitiveVisual::SinglePrimitiveVisual(std::unique_ptr<Primitive> primitive){
    m_primitives.push_back(std::move(primitive));
}

void SinglePrimitiveVisual::Serialize( IStreamBase& stream ){
    // This visual doesn't support serialization for now
    sAssert(false, GENERAL);
}

void SinglePrimitiveVisual::ApplyTransform( const Transform& transform ){
    // This is not currently needed for now
    sAssert(false, GENERAL);
}

#if INTEL_EMBREE_ENABLED
void Visual::BuildEmbreeGeometry(RTCDevice device, Embree& embree) const{
    for (const auto& primitive : m_primitives)
        primitive->BuildEmbreeGeometry(device, embree);
}

void MeshVisual::BuildEmbreeGeometry(RTCDevice device, Embree& embree) const{
    // if there is nothing in this mesh, just bail early.
    if(!m_memory)
        return;

    // ideally, there should be multiple geometries, not just one.
    auto geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    // get the vertex buffer and index buffer sizes
    const auto vert_cnt = m_memory->m_vertices.size();
    const auto indices_cnt = m_memory->m_indices.size();
    const auto vert_stride = sizeof(float) * 3;
    const auto index_stride = sizeof(unsigned int) * 3;

    if (vert_cnt == 0 || indices_cnt == 0)
        return;

    auto embree_geom = std::make_unique<EmbreeGeometry>();

    auto vertices = (float*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vert_stride, vert_cnt);
    auto indices = (unsigned int*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, index_stride, indices_cnt);
    
    // fill vertex buffer
    auto i = 0;
    for(const auto& vert : m_memory->m_vertices){
        const auto offset = 3 * i++;
        vertices[offset] = vert.m_position.x;
        vertices[offset + 1] = vert.m_position.y;
        vertices[offset + 2] = vert.m_position.z;
    }
    sAssert(i == vert_cnt, SPATIAL_ACCELERATOR);

    i = 0;
    for(const auto& index : m_memory->m_indices){
        const auto offset = 3 * i++;
        indices[offset] = index.m_id[0];
        indices[offset + 1] = index.m_id[1];
        indices[offset + 2] = index.m_id[2];
    }
    sAssert(i == indices_cnt, SPATIAL_ACCELERATOR);
    
    // we are done with creating the geometry
    rtcCommitGeometry(geometry);

    // due to native support of Embree, just one geometry is enough for MeshVisual
    embree_geom->m_geometry = geometry;

    // copy the triangle list
    for(const auto& triangle : m_primitives)
        embree_geom->m_primitives.push_back(triangle.get());

    // we are done creating the geomtry, push it in the spatial data structure.
    embree.PushGeometry(std::move(embree_geom));
}

#endif