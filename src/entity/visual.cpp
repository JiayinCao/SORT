/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "visual.h"
#include "managers/matmanager.h"
#include "core/scene.h"
#include "shape/triangle.h"
#include "core/primitive.h"

IMPLEMENT_RTTI( MeshVisual );
IMPLEMENT_RTTI( HairVisual );

void MeshVisual::FillScene( Scene& scene ){
    for (const auto& mi : m_memory->m_indices){
        m_triangles.push_back( std::make_unique<Triangle>( this , mi ) );
        scene.AddPrimitive( std::make_unique<Primitive>( mi.m_mat , m_triangles.back().get() ) );
    }
}

void MeshVisual::Serialize( IStreamBase& stream ){
    m_memory = std::make_unique<BufferMemory>();
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
        scene.AddPrimitive( std::make_unique<Primitive>( mat , line.get() ) );
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
        
        const auto width_delta = ( width_bottom - width_tip ) / (float)hair_step;
        const auto v_delta = 1.0f / ( float ) hair_step;
        Point prevP;
        auto prevV = 0.0f;

        auto width = width_bottom;
        auto v = 0.0f;
        for( auto j = 0u ; j <= hair_step ; ++j ){
            Point curP;
            stream >> curP;

            if( j > 0 ){
                // Prevent float precision issue cauing negative width
                const auto width_start = width * 0.5f;
                const auto width_end = std::max( 0.0f , width - width_delta ) * 0.5f;
                const auto v_start = v;
                const auto v_end = std::min( 1.0f , v + v_delta );

                m_lines.push_back(std::make_unique<Line>( prevP , curP , v_start , v_end , width_start, width_end , mat_id ) );
                width -= width_delta;
                v += v_delta;
            }
            prevP = curP;
        }
    }
}

void HairVisual::ApplyTransform( const Transform& transform ){
    for( auto& line : m_lines )
        line->SetTransform( transform );
}