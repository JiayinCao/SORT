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
    for (const MeshIndex& mi : m_memory->m_indices){
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
    for( const auto& line : m_lines )
        scene.AddPrimitive( std::make_unique<Primitive>( nullptr , line.get() ) );
        return;
    Point p0( 0.0f , 0.0f , 0.0f );
    Point p1( 0.0f , 5.0f , 0.0f );
    m_lines.push_back(std::make_unique<Line>( p0 , p1 , m_width0 , m_width1 ) );
    scene.AddPrimitive( std::make_unique<Primitive>( nullptr , m_lines.back().get() ) );
}

void LineSetVisual::Serialize( IStreamBase& stream ){
    stream >> m_width0;
    stream >> m_width1;
    unsigned int cnt = 0;
    stream >> cnt;
    for( int i = 0 ; i < cnt ; ++i ){
        Point p0 , p1;
        stream >> p0 >> p1;
        m_lines.push_back(std::make_unique<Line>( p0 , p1 , m_width0 , m_width1 ) );
    }
}

void LineSetVisual::ApplyTransform( const Transform& transform ){
    for( auto& line : m_lines )
        line->SetTransform( transform );
}