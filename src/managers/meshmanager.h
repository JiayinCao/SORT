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

#pragma once

#include "core/define.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include "core/singleton.h"
#include "core/enum.h"
#include "core/define.h"
#include "math/point.h"
#include "math/vector3.h"
#include "math/transform.h"
#include "material/material.h"
#include "stream/stream.h"

struct MeshVertex {
    Point       m_position;
    Vector      m_normal;
    Vector      m_tangent;
    Vector2f    m_texCoord;
};

struct MeshIndex {
    int        m_id[3] = { -1 };   /**< Indices for one triangle. */
    Material*  m_mat = nullptr;    /**< Materials attached to the triangle. */
};

//! @brief  A wrapper for mesh information.
//!
//! Instead of using obj style memory layout, an approach that is similar to vertex buffer and index buffer
//! in real time rendering is used here. An array of struct is used here for better cache performance during
//! rendering.
//! The old solution was to isolate the position, normal and texture coordinate buffers so that we can try
//! saving minimal information without duplicating unnecessary data. However, the effort to generate such a
//! layout of date requires quite some time in Blender, due to which reason, it was deprecated.
class BufferMemory : public SerializableObject
{
public:
    std::vector<MeshVertex> m_vertices;         /**< Vertex information including position, normal and etc.*/
    std::vector<MeshIndex>  m_indices;          /**< Index information of the mesh, there is also material id in it. */
    bool                    m_hasUV = false;    /**< Whether the mesh has UV information. */

    // generate UV coordinate
    void    GenUV();

	// apply transform
	void    ApplyTransform( const Transform& );

	// generate tangent for the triangle mesh
	void	GenSmoothTagent();

	// serialization interface for BufferMemory
    void    Serialize( IStreamBase& stream ) override;

private:
    Vector	_genTagentForTri( const MeshIndex& ) const;
};