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

#pragma once

#include "core/define.h"
#include <vector>
#include <memory>
#include "math/point.h"
#include "math/vector3.h"
#include "math/transform.h"
#include "stream/stream.h"
#include "medium/mediumdata.h"

class MaterialBase;

//! @brief  MeshVertex defines the basic information for a vertex in mesh.
struct MeshVertex {
    Point       m_position;     /**< The position of the vertex in world space. */
    Vector      m_normal;       /**< The normal of the vertex in world space. */
    Vector      m_tangent;      /**< The tangent of the vertex in world space. */
    Vector2f    m_texCoord;     /**< The only channel of texture coordinate of the vertex. */
};

//! @brief  MeshFaceIndex defines the indices of the three vertices and also the material index of the face.
struct MeshFaceIndex {
    int                     m_id[3] = { -1 };   /**< Indices for one triangle. */
    const MaterialBase*     m_mat = nullptr;    /**< Materials attached to the triangle. */
};

//! @brief  A wrapper for mesh information.
//!
//! Instead of using obj style memory layout, an approach that is similar to vertex buffer and index buffer
//! in real time rendering is used here. An array of struct is used here for better cache performance during
//! rendering.
//! The old solution was to isolate the position, normal and texture coordinate buffers so that we can try
//! saving minimal information without duplicating unnecessary data. However, the effort to generate such a
//! layout of date requires quite some time in Blender, due to which reason, it was deprecated.
class Mesh : public SerializableObject{
public:
    std::vector<MeshVertex>     m_vertices;         /**< Vertex information including position, normal and etc.*/
    std::vector<MeshFaceIndex>  m_indices;          /**< Index information of the mesh, there is also material id in it. */
    bool                        m_hasUV = false;    /**< Whether the mesh has UV information. */

    //! @brief      Generate UV coordinate for the vertices.
    void    GenUV();

    //! @brief      Apply transformation.
    //!
    //! Triangle is the most common shape used in rendering a scene. Unlike other shape in SORT, which transform the
    //! ray to local space, triangle vertices are pre-transformed to world space for better performance.
    void    ApplyTransform( const Transform& );

    //! @brief      Generate tangent for the triangle mesh.
    void    GenSmoothTagent();

    //! @brief      Serializing data from stream.
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation,
    //!             it could come from different places.
    void    Serialize( IStreamBase& stream ) override;

    //! @brief      Sample volume density
    //!
    //! For meshes that don't have volume inside, this function should not even be called.
    //! However, if it is, which is totally possible depending on data setup, it returns 0.0f.
    //!
    //! @param  pos     Position in world space.
    //! @return         The density of the volume.
    float       SampleVolumeDensity(const Point& pos) const;

    //! @brief      Sample volume color
    //!
    //! @param  pos     Position in world space.
    //! @return         The color of the volume.
    Spectrum    SampleVolumeColor(const Point& pos) const;

private:
    //! @brief      Generate tangent for the triangles.
    //!
    //! @return     Generated tangent.
    Vector      genTagentForTri( const MeshFaceIndex& ) const;

    /**< Transformation from world space to local volume space. */
    mutable float   m_world2LocalVolume[16] = { 0.0f };

    /**< Tranform from object local space to volume texture space. */
    Matrix          m_local2Volume;
    /**< Tranfrom from world space to volume texture space. */
    Matrix          m_world2Volume;

    /**< Whether the world2localvolume matrix is cached or not. */
    mutable bool    m_w2lvCached = false;

    /**< The density of volume data insize this mesh. */
    std::unique_ptr<MediumDensity>  m_volumeDensity;
    /**< The color of the volume data inside this mesh. */
    std::unique_ptr<MediumColor>    m_volumeColor;
};