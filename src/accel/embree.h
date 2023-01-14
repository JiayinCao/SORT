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

#ifdef INTEL_EMBREE_ENABLED

// Intel embree API
#include <embree3/rtcore.h>
#include "accelerator.h"
#include "embree_util.h"

//! @brief  A thin wrapper of Intel Embree
/**
 * This is a thin wrapper of Intel Embree.
 * It does nothing but to delegate all the tasks to Intel Embree system.
 * For a detailed introduction of Embree, refer to this page
 * https://www.embree.org/index.html
 */
class Embree : public Accelerator{
public:
    DEFINE_RTTI(Embree, Accelerator);
    
    //! @brief      Default constructor
    Embree();

    //! @brief      Destructor
    ~Embree();

    //! @brief      Get intersection between the ray and the primitive set using KD-Tree.
    //!
    //! It will return true if there is intersection between the ray and the primitive set.
    //! In case of an intersection, it will fill the structure and return the nearest intersection.
    //! This intersection could possibly be a fully transparent intersection, it is up to the higher
    //! level logic to handle (semi)transparency later.
    //!
    //! @param r            The input ray to be tested.
    //! @param intersect    The intersection result. If a nullptr pointer is provided,
    //!                     it stops as long as it finds an intersection. It is faster
    //!                     than the one with intersection information data and suitable
    //!                     for shadow ray calculation.
    //! @return             It will return true if there is an intersection, otherwise
    //!                     it returns false.
    bool GetIntersect( RenderContext& rc, const Ray& r , SurfaceInteraction& intersect ) const override;

#ifndef ENABLE_TRANSPARENT_SHADOW
    //! @brief This is a dedicated interface for detecting shadow rays.
    //!
    //! Instead of merging the interface with 'GetIntersect', this is a separate interface purely for occlusion detection.
    //! There is a need for it so that we can achieve better performance. There will be less branch in this interfaces and
    //! most importantly the traversed node doesn't need to be sorted.
    //!
    //! @param r            The ray to be tested.
    //! @return             Whether the ray is occluded by anything.
    bool IsOccluded( const Ray& r ) const override;
#endif

    //! @brief Build acceleration structure
    //!
    //! @param primitives       A vector holding all primitives.
    //! @param bbox             The bounding box of the scene.
    void    Build(const Scene& scene) override;

    //! @brief      Serializing data from stream, this data structure is not configurable in Blender.
    //!
    //! @param      Stream where the serialization data comes from. Depending on different
    //!             situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override{}

    //! @brief    Clone the accelerator.
    //!
    //! Only configuration will be cloned, not the data inside the accelerator, this is for primitives that has volumes attached.
    //!
    //! @return         Cloned accelerator.
    std::unique_ptr<Accelerator>    Clone() const override;

    //! @brief      Push an Embree geometry.
    //!
    //! @param  geom    Embree geometry
    //! @return         Geometry id.
    unsigned int    PushGeometry(std::unique_ptr<EmbreeGeometry> geom);

private:
    /**< Embree device. */
    RTCDevice   m_rtc_device;

    /**< Embree scene. */
    RTCScene    m_rtc_scene;

    // temporary data structure
    std::vector<const Primitive*> m_temp;

    // a list of embree geometry
    std::vector<std::unique_ptr<EmbreeGeometry>>   m_geometries;

    SORT_STATS_ENABLE( "Spatial-Structure(Embree)" )
};

#endif