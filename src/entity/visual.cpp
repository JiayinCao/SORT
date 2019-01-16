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
IMPLEMENT_RTTI( LineSetVisual );

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

void LineSetVisual::FillScene( Scene& scene ){
    for( const auto& line : m_lines ){
        auto mat = MatManager::GetSingleton().GetMaterial(line->GetMaterialId());
        scene.AddPrimitive( std::make_unique<Primitive>( mat , line.get() ) );
    }
}

void LineSetVisual::Serialize( IStreamBase& stream ){
    auto vert_cnt = 0u;
    stream >> vert_cnt;
    for( auto i = 0u ; i < vert_cnt ; ++i ){
        m_vertices.push_back( Point() );
        stream >> m_vertices.back();
    }

    auto line_cnt = 0u;
    stream >> line_cnt;
    for( auto i = 0u ; i < line_cnt ; ++i ){
        unsigned int id0 , id1;
        stream >> id0 >> id1;
        float w0 , w1;
        stream >> w0 >> w1;
        int matId = -1;
        stream >> matId;
        m_lines.push_back(std::make_unique<Line>( m_vertices[id0] , m_vertices[id1] , w0 * 0.5f , w1 * 0.5f , matId ) );
    }
}

void LineSetVisual::ApplyTransform( const Transform& transform ){
    for( auto& line : m_lines )
        line->SetTransform( transform );
}