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

#include <numeric>
#include "visual.h"
#include "material/matmanager.h"
#include "core/scene.h"

void MeshVisual::FillScene( Scene& scene ){
    for (const auto& mi : m_memory->m_indices){
        m_triangles.push_back( std::make_unique<Triangle>( this , mi ) );
        m_primitives.push_back(std::make_unique<Primitive>(m_memory.get(), mi.m_mat, m_triangles.back().get()));
        scene.AddPrimitive(m_primitives.back().get());
    }
}

void MeshVisual::Serialize( IStreamBase& stream ){
    m_memory = std::make_unique<Mesh>();
    m_memory->Serialize(stream);
}

void MeshVisual::ApplyTransform( const Transform& transform ){
    m_memory->ApplyTransform( transform );
    m_memory->GenUV();
    m_memory->GenSmoothTagent();
}

void HairVisual::FillScene( Scene& scene ){
    for( const auto& line : m_lines ){
        auto mat = MatManager::GetSingleton().GetMaterial(line->GetMaterialId());
        m_primitives.push_back(std::make_unique<Primitive>(nullptr, mat, line.get()));
        scene.AddPrimitive(m_primitives.back().get());
    }
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

            prev_w = cur_w;
            prev_v = cur_v;
        }
    }
}

void HairVisual::ApplyTransform( const Transform& transform ){
    for( auto& line : m_lines )
        line->SetTransform( transform );
}
