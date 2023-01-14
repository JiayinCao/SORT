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

#pragma once

#if INTEL_EMBREE_ENABLED
#include <embree3/rtcore.h>
#endif

#include "core/rtti.h"
#include "core/mesh.h"
#include "shape/triangle.h"
#include "shape/line.h"
#include "core/primitive.h"
#include "accel/embree_util.h"

class Embree;

//! @brief Visual is the container for a specific type of shape that can be seen in SORT.
/**
 * Visual could be a single shape, like sphere, triangle. It could also be a set of triangles,
 * triangle mesh. Basically, visual is the most fundamental unit in SORT that is visible.
 */
class Visual : public SerializableObject {
public:
    //! @brief  Some visual will apply transformation earlier for better performance.
    //!
    //! @param  transform   The transform of the visual to be applied.
    virtual void        ApplyTransform( const Transform& transform ) = 0;

    //! @brief  Get number of primitives in this visual
    unsigned            GetPrimitiveCount() const{
        return (unsigned)m_primitives.size();
    }

    #if INTEL_EMBREE_ENABLED
        //! @brief  Process embree data.
        virtual void BuildEmbreeGeometry(RTCDevice device, Embree& embree) const;
    #endif
    
protected:
    /*< Primitives that shape the visual. */
    std::vector<std::unique_ptr<Primitive>>  m_primitives;

    friend class ScenePrimitiveIterator;
};

//! @brief Triangle Mesh Visual.
/**
 * MeshVisual is the most common Visual in a ray tracer. It is composited with a set of
 * triangles. Most of the objects in a scene uses this visual.
 */
class MeshVisual : public Visual{
public:
    DEFINE_RTTI( MeshVisual , Visual );

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

    #if INTEL_EMBREE_ENABLED
        //! @brief  Process embree data.
        //!
        //! Triangle is the most common primitive used in ray tracing.
        //! Unlike other types, it supports one single Embree geometry with
        //! a souple of triangles.
        void BuildEmbreeGeometry(RTCDevice device, Embree& embree) const override;
    #endif

public:
    /**< Memory for the mesh. */
    std::unique_ptr<Mesh>                       m_memory;
    /**< This is to make sure the memory of triangles will be properly cleared. */
    std::vector<std::unique_ptr<Triangle>>      m_triangles;
};

//! HairVisual has a bunch of lines.
/**
 * Just like MeshVisual may have lots of triangles, HairVisual has loads of line shape in it.
 * This visual is usually used to represent things like fur or hair.
 */
class HairVisual : public Visual{
public:
    DEFINE_RTTI( HairVisual , Visual );

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
};

//! This is currently only used for area light for now.
/*
 * SinglePrimitiveVisual only has one single primitive in it.
 */
class SinglePrimitiveVisual : public Visual{
public:
    //! @brief This is the only way to push a primitive in this visual.
    SinglePrimitiveVisual(std::unique_ptr<Primitive> primitive);

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! This should be empty since this visual is not supposed to be serialized from IO.
    //!
    //! @param  stream      Input stream for data.
    void        Serialize( IStreamBase& stream ) override;

    //! @brief  Some visual will apply transformation earlier for better performance.
    //!
    //! @param  transform   The transform of the visual to be applied.
    void        ApplyTransform( const Transform& transform ) override;
};