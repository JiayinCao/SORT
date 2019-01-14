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

#include "managers/meshmanager.h"
#include "shape/triangle.h"
#include "shape/line.h"

//! @brief Visual is the container for a specific type of shape that can be seen in SORT.
/**
 * Visual could be a single shape, like sphere, triangle. It could also be a set of triangles,
 * triangle mesh. Basically, visual is the most fundamental unit in SORT that is visible.
 */
class Visual : public SerializableObject {
public:
    //! @brief  Fill the scene with triangles.
    //!
    //! @param  scene       The scene to be filled.
    virtual void        FillScene( class Scene& scene ) = 0;

    //! @brief  Some visual will apply transformation earlier for better performance.
    //!
    //! @param  transform   The transform of the visual to be applied.
    virtual void        ApplyTransform( const Transform& transform ) = 0;
};

//! @brief Triangle Mesh Visual.
/**
 * MeshVisual is the most common Visual in a ray tracer. It is composited with a set of
 * triangles. Most of the objects in a scene uses this visual.
 */
class MeshVisual : public Visual{
public:
    DEFINE_RTTI( MeshVisual , Visual );

    //! @brief  Fill the scene with triangles.
    //!
    //! @param  scene       The scene to be filled.
    void        FillScene( class Scene& scene ) override;

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the visual. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void        Serialize( IStreamBase& stream ) override;

    //! @brief  Some visual will apply transformation earlier for better performance.
    //!
    //! @param  transform   The transform of the visual to be applied.
    void        ApplyTransform( const Transform& transform ) override;

public:
    /**< Memory for the mesh. */
    std::unique_ptr<BufferMemory>               m_memory;
    /**< This is to make sure the memory of triangles will be properly cleared. */
    std::vector<std::unique_ptr<Triangle>>      m_triangles;
};

//! LineSetVisual has a bunch of lines.
/**
 * Just like MeshVisual may have lots of triangles, LineSetVisual has loads of line shape in it.
 * This visual is usually used to represent things like fur or hair.
 */
class LineSetVisual : public Visual{
public:
    DEFINE_RTTI( LineSetVisual , Visual );

    //! @brief  Fill the scene with triangles.
    //!
    //! @param  scene       The scene to be filled.
    void        FillScene( class Scene& scene ) override;

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the visual. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void        Serialize( IStreamBase& stream ) override;

    //! @brief  Some visual will apply transformation earlier for better performance.
    //!
    //! @param  transform   The transform of the visual to be applied.
    void        ApplyTransform( const Transform& transform ) override;
    
private:
    /**< Memory container holding the lines. */
    std::vector<std::unique_ptr<Line>>  m_lines;
    /**< Width at the first point of the lines. */
    float       m_width0 = 0.0f;
    /**< Width at the second point of the line. */
    float       m_width1 = 0.0f;
};