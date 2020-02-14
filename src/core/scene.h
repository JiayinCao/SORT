/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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
#include "core/sassert.h"
#include "math/bbox.h"
#include "spectrum/spectrum.h"
#include "math/interaction.h"
#include "math/transform.h"
#include "camera/camera.h"
#include "entity/entity.h"
#include "core/primitive.h"
#include "core/samplemethod.h"

class Light;
struct BSSRDFIntersections;

//! @brief  Data structure representing the whole scene.
/**
 * Scene is responsible for maintaining all of the lifetime of its own data structure.
 */
class   Scene{
public:
    //! @brief Serialize scene from stream.
    //!
    //! @param  stream      The streaming source where scene information is loaded from.
    //! @return             Whether the scene is loaded correctly.
    bool    LoadScene( class IStreamBase& stream );

    //! @brief  Find the first intersection between a ray and the whole scene.
    //!
    //! @param  intersect   Intersection information at exitant point.
    //! @param  r           The ray to be tested.
    //! @param  intersect   The result where the intersected information is to be returned.
    //! @return             Whether there is an intersection between the ray and the scene.
    bool    GetIntersect( const Ray& r , SurfaceInteraction& intersect ) const;

#ifndef ENABLE_TRANSPARENT_SHADOW
    //! @brief  This is a dedicated interface for detecting shadow rays.
    //!
    //! Instead of merging the interface with 'GetIntersect', this is a separate interface purely for occlusion detection.
    //! There is a need for it so that we can achieve better performance. There will be less branch in this interfaces and
    //! most importantly the traversed node doesn't need to be sorted.
    //!
    //! @param r            The ray to be tested.
    //! @return             Whether the ray is occluded by anything.
    bool    IsOccluded(const Ray& r) const;
#else
    //! @brief  Evaluate occlusion along a ray segment.
    //!
    //! This doesn't normaly happen in real life. But with the introduction of transaprent BSDF, it is innevitable to support
    //! shadow attenuation forcing shadowing information transform from binary to a range.
    //! The returned value is the spectrum dependent percentage of un-occluded radiance. Put it in other words, 0 means fully
    //! occluded, 1.0 means fully un-occluded.
    //!
    //! @param  r           The ray to be tested.
    //! @param  ms          The medium stack to be passed in. Medium aware integrator needs to pass non-empty pointer.
    //! @return             The occlusion along the ray.
    Spectrum    GetAttenuation( const Ray& r , MediumStack* ms = nullptr ) const;
#endif

    //! @brief Get multiple intersections between the ray and the primitive set using spatial data structure.
    //!
    //! This is a specific interface designed for SSS during disk ray casting. Without this interface, the algorithm has to use the
    //! above one to acquire all intersections in a brute force way, which obviously introduces quite some duplicated work.
    //! The intersection returned doesn't guarrantee the order of the intersection of the results, but it does guarrantee to get the
    //! nearest N intersections.
    //!
    //! @param  r           The input ray to be tested.
    //! @param  intersect   The intersection result that holds all intersectionn.
    //! @param  matID       We are only interested in intersection with the same material, whose material id should be set to matID.
    void    GetIntersect( const Ray& r , BSSRDFIntersections& intersect , const StringID matID = INVALID_SID ) const;

    // get light
    const Light* GetLight( unsigned i ) const{
        sAssert( i < m_lights.size() , LIGHT );
        return m_lights[i];
    }
    // add light
    void AddLight( Light* light );
    // get lights
    const std::vector<Light*>& GetLights() const {return m_lights;}
    // get sky light
    const Light* GetSkyLight() const {return m_skyLight;}
    // set sky light
    void SetSkyLight(Light* light){
        m_skyLight = light;
    }
    // get sampled light
    const Light* SampleLight( float u , float* pdf ) const;
    // get the properbility of the sample
    float LightProperbility( unsigned i ) const;
    // get the number of lights
    unsigned LightNum() const{
        return (unsigned)m_lights.size();
    }
    // get bounding box of the scene
    const BBox& GetBBox() const;
    // add primitives
    void AddPrimitive( const Primitive* primitive) {
        m_primitives.push_back( std::move(primitive) );
    }
    // get primtives
    const std::vector<const Primitive*>*   GetPrimitives() const {
        return &m_primitives;
    }

    // Evaluate sky
    Spectrum    Le( const Ray& ray ) const;

    // Setup scene camera
    void SetupCamera(Camera* camera) {
        m_camera = camera;
    }
    // Get camera from the scene
    Camera* GetCamera() const {
        return m_camera;
    }

private:
    std::vector<std::unique_ptr<Entity>>        m_entities;             /**< Entities in the scene. */
    std::vector<Light*>                         m_lights;               /**< Lights in the scene. */

    std::vector<const Primitive*>               m_primitives;           /**< A list holding all primitives. */
    std::vector<const Primitive*>               m_volPrimitives;        /**< A list holding all primitives that has volume attached to it. */

    Light*                  m_skyLight = nullptr;   /**< Sky light if available. */
    Camera*                 m_camera = nullptr;     /**< Camera of the scene. */

    /**< distribution of light power */
    std::unique_ptr<Distribution1D>             m_lightsDis = nullptr;

    // bounding box for the scene
    mutable BBox    m_BBox;

    // generate primitive buffer
    void    _generatePriBuf();

    // compute light cdf
    void    _genLightDistribution();

    friend class MeshVisual;
};