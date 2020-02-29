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

#include <vector>
#include <string>
#include <OSL/oslexec.h>
#include "stream/stream.h"
#include "osl_system.h"

struct SurfaceInteraction;
struct MediumInteraction;
class ScatteringEvent;
class MediumStack;

//! @brief  Base interface for material.
/**
 * MaterialBase is the basic interface for materials.
 */
class MaterialBase : public SerializableObject {
public:
    //! @brief      Parse scattering event from the material shader.
    //!
    //! @param      se              Scattering event to be returned.
    virtual void       UpdateScatteringEvent(ScatteringEvent& se) const = 0;

    //! @brief      Parse volume from the material shader.
    //!
    //! @param      mi              Interaction with the medium.
    //! @param      flag            A flag indicates whether to add or remove the medium.
    //! @param      ms              Medium stack to be populated.
    virtual void       UpdateMediumStack(const MediumInteraction& mi, const SE_Interaction flag, MediumStack& ms) const = 0;

    //! @brief      Take sample in a medium at a specific position.
    //!
    //! This is specifically for sampling the heteregenous sampling. Neither of the other two types of volumes need take random samples.
    //!
    //! @param      mi              Interaction with the medium.
    //! @param      ms              Medium sample taken.
    virtual void       EvaluateMediumSample(const MediumInteraction& mi, MediumSample& ms) const = 0;

    //! @brief      Evaluate translucency.
    //!
    //! @param      intersection    The intersection.
    //! @return                     The transparency at the intersection.
    virtual Spectrum   EvaluateTransparency(const SurfaceInteraction& intersection) const = 0;

    //! @brief  Build shader in OSL.
    //!
    //! @param  shadingSys      Open-Shading-Language shading system.
    virtual void       BuildMaterial() = 0;

    //! @brief  Get material ID.
    //!
    //! @return     Material ID that uniquely identifies the material.
    virtual StringID   GetUniqueID() const = 0;

    //! @brief  Whether the material has transparency
    //!
    //! @return Return true if there is transparency in the material.
    virtual bool       HasTransparency() const = 0;

    //! @brief  Whether the material has sss
    //!
    //! @return Return true if there is sss node in the material.
    virtual bool       HasSSS() const = 0;

    //! @brief  Whether the material is attached with a volume.
    //!
    //! @return Return true if the material is attached with a volume.
    virtual bool HasVolumeAttached() const = 0;
};

//! @brief  A thin layer of material definition.
/**
 * SORT supports node-graph based material system so that it could be flexible enough to support varies features.
 * This is no pre-defined material in SORT. Every material is a combination of material nodes, the topology
 * of the tree defines the behavior of the material. With this design, it is very easy to drive different parameters by
 * textures or any other information. For invalid material node graph tree, a white default material is returned as default.
 * The implementation of material system heavily depends on Open Shading Language, an open-source project own by Sony Pictures
 * Imageworks, following is the link to the github page to access its source code.
 * https://github.com/imageworks/OpenShadingLanguage
 */
class Material : public MaterialBase {
public:
    //! @brief      Parse scattering event from the material shader.
    //!
    //! @param      se              Scattering event to be returned.
    void        UpdateScatteringEvent( ScatteringEvent& se ) const override;

    //! @brief      Parse volume from the material shader.
    //!
    //! @param      mi              Interaction with the medium.
    //! @param      flag            A flag indicates whether to add or remove the medium.
    //! @param      ms              Medium stack to be populated.
    void        UpdateMediumStack(const MediumInteraction& mi, const SE_Interaction flag, MediumStack& ms) const override;

    //! @brief      Take sample in a medium at a specific position.
    //!
    //! This is specifically for sampling the heteregenous sampling. Neither of the other two types of volumes need take random samples.
    //!
    //! @param      mi              Interaction with the medium.
    //! @param      ms              Medium sample taken.
    void        EvaluateMediumSample(const MediumInteraction& mi, MediumSample& ms) const override;

    //! @brief      Evaluate translucency.
    //!
    //! @param      intersection    The intersection.
    //! @return                     The transparency at the intersection.
    Spectrum    EvaluateTransparency( const SurfaceInteraction& intersection ) const override{
        // this should happen most of the time in the absence of transparent node.
        if (!m_hasTransparentNode)
            return 0.0f;
        return m_special_transparent ? 1.0f : (::EvaluateTransparency(m_surface_shader.get(), intersection));
    }

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void        Serialize(IStreamBase& stream) override;

    //! @brief  Build shader in OSL.
    //!
    //! @param  shadingSys      Open-Shading-Language shading system.
    void        BuildMaterial() override;

    //! @brief  Get material ID.
    //!
    //! @return     Material ID that uniquely identifies the material.
    StringID    GetUniqueID() const override {
        return m_matID;
    }

    //! @brief  Whether the material has transparency
    //!
    //! @return Return true if there is transparency in the material.
    bool        HasTransparency() const override {
        return m_hasTransparentNode;
    }
    
    //! @brief  Whether the material has sss
    //!
    //! @return Return true if there is sss node in the material.
    bool        HasSSS() const override {
        return m_hasSSSNode;
    }

    //! @brief  Whether the material is attached with a volume.
    //!
    //! @return Return true if the material is attached with a volume.
    bool        HasVolumeAttached() const override {
        return m_volume_shader_valid;
    }

    //! @brief  Get volume ray matching step size.
    //!
    //! @return Ray marching step size.
    SORT_FORCEINLINE float GetVolumeStep() const {
        return m_volumeStep;
    }

    //! @brief  Get volume ray marching max step count.
    //!
    //! @return Maximum steps to march during ray marching.
    SORT_FORCEINLINE unsigned int GetVolumeStepCnt() const {
        return m_volumeStepCnt;
    }

private:
    /**< Whether this is a valid material */
    bool                            m_surface_shader_valid = false;
    bool                            m_volume_shader_valid = false;

    /**< In the case where there is volume but no surface material, transparent will be automatically attached as surface material. */
    bool                            m_special_transparent = false;

    /**< Unique name of the material. */
    std::string                     m_name;
    /**< Material id. */
    StringID                        m_matID = INVALID_SID;

    struct ShaderSource{
        std::string name;
        std::string type;
        std::string source;
    };
    struct ShaderConnection{
        std::string source_shader , source_property;
        std::string target_shader , target_property;
    };

    struct OSL_ShaderData {
        /**< Shader source code. */
        std::vector<ShaderSource>       m_sources;
        /**< Shader connections. */
        std::vector<ShaderConnection>   m_connections;
    };

    /**< OSL shader source data. */
    OSL_ShaderData                  m_surface_shader_data;
    OSL_ShaderData                  m_volume_shader_data;

    /**< OSL device surface shader. */
    OSL::ShaderGroupRef             m_surface_shader = nullptr;
    OSL::ShaderGroupRef             m_volume_shader = nullptr;

    bool                            m_hasTransparentNode = false;
    bool                            m_hasSSSNode = false;

    float                           m_volumeStep = 0.1f;
    unsigned int                    m_volumeStepCnt = 1024;
};

//! @brief  MaterialProxy is nothing but a thin wrapper of another existed material.
/**
 * MaterialProxy only directs all methods to its referred material. The existence of MaterialProxy
 * is to differentiate the same material used in different meshes, which is fairly important in SSS 
 * and volumetric rendering.
 */
class MaterialProxy : public MaterialBase {
public:
    //! @brief  Constructor of material proxy taking a reference of its referred material.
    //!
    //! @param  material    Material to be referred.
    MaterialProxy(const MaterialBase& material) : m_material(material) {}

    //! @brief      Parse scattering event from the material shader.
    //!
    //! @param      se              Scattering event to be returned.
    void       UpdateScatteringEvent(ScatteringEvent& se) const override;

    //! @brief      Parse volume from the material shader.
    //! @param      mi              Interaction with the medium.
    //! @param      flag            A flag indicates whether to add or remove the medium.
    //! @param      ms              Medium stack to be populated.
    void       UpdateMediumStack(const MediumInteraction& mi, const SE_Interaction flag, MediumStack& ms) const override;

    //! @brief      Take sample in a medium at a specific position.
    //!
    //! This is specifically for sampling the heteregenous sampling. Neither of the other two types of volumes need take random samples.
    //!
    //! @param      mi              Interaction with the medium.
    //! @param      ms              Medium sample taken.
    void        EvaluateMediumSample(const MediumInteraction& mi, MediumSample& ms) const override;

    //! @brief  Just an empty interface, there is no serialization support for this type of material.
    //!
    //! @param  stream      Input stream for data.
    void       Serialize(IStreamBase& stream) override {}

    //! @brief      Evaluate translucency.
    //!
    //! @param      intersection    The intersection.
    //! @return                     The transparency at the intersection.
    Spectrum   EvaluateTransparency(const SurfaceInteraction& intersection) const override;

    //! @brief  This should be an empty method that does nothing
    //!
    //! @param  shadingSys      Open-Shading-Language shading system.
    void       BuildMaterial() override {}

    //! @brief  Get material ID.
    //!
    //! @return     Material ID that uniquely identifies the material.
    StringID   GetUniqueID() const override;

    //! @brief  Whether the material has transparency
    //!
    //! @return Return true if there is transparency in the material.
    bool       HasTransparency() const override;

    //! @brief  Whether the material has sss
    //!
    //! @return Return true if there is sss node in the material.
    bool       HasSSS() const override;

    //! @brief  Whether the material is attached with a volume.
    //!
    //! @return Return true if the material is attached with a volume.
    bool       HasVolumeAttached() const override;

private:
    /**< Material to be referred. */
    const MaterialBase& m_material;
};