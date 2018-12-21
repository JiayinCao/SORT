/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

void MeshVisual::FillScene( Scene& scene ){
    unsigned base = (unsigned)scene.m_primitiveBuf.size();

	// generate the triangles
	unsigned trunkNum = (unsigned)m_memory->m_TrunkBuffer.size();
	for( unsigned i = 0 ; i < trunkNum ; i++ )
	{
		unsigned trunkTriNum = (unsigned)m_memory->m_TrunkBuffer[i].m_IndexBuffer.size() / 3;
		for( unsigned k = 0 ; k < trunkTriNum ; k++ ){
			std::shared_ptr<Triangle> tri = std::make_shared<Triangle>( this , &(m_memory->m_TrunkBuffer[i].m_IndexBuffer[3*k]) );
			std::shared_ptr<Material> mat = m_memory->m_TrunkBuffer[i].m_mat ? m_memory->m_TrunkBuffer[i].m_mat : MatManager::GetSingleton().GetDefaultMat();
			scene.m_primitiveBuf.push_back( new Primitive( mat , tri ) );
		}
		base += (unsigned)(m_memory->m_TrunkBuffer[i].m_IndexBuffer.size() / 3);
	}
}

void MeshVisual::Serialize( IStreamBase& stream ){
	m_memory->Serialize(stream);
}

void MeshVisual::Serialize( OStreamBase& stream ){
	m_memory->Serialize(stream);
}