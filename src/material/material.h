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

class Intersection;
class ScatteringEvent;

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
class Material : public SerializableObject{
public:
    //! @brief      Parse scattering event from the material shader.
    //!
    //! @param      se              Scattering event to be returned.
    void        UpdateScatteringEvent( ScatteringEvent& se ) const;

    //! @brief      Evaluate translucency.
    //!
    //! @param      intersection    The intersection.
    //! @return                     The transparency at the intersection.
    SORT_FORCEINLINE Spectrum    EvaluateTransparency( const Intersection& intersection ) const{
        // this should happen most of the time in the absence of transparent node.
        if( !m_hasTransparentNode )
            return 0.0f;
        return ::EvaluateTransparency( m_shader.get() , intersection );
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
    //! @return                 Whether the shader is created successfully.
    bool        BuildMaterial();

    //! @brief  Get material ID.
    //!
    //! @return     Material ID that uniquely identifies the material.
    SORT_FORCEINLINE StringID    GetID() const {
        return m_matID;
    }

private:
    /**< Whether this is a valid material */
    bool                            m_valid = false;
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

    /**< Shader source code. */
    std::vector<ShaderSource>       m_sources;
    /**< Shader connections. */
    std::vector<ShaderConnection>   m_connections;

    /**< OSL device shader. */
    OSL::ShaderGroupRef             m_shader = nullptr;

    bool                            m_hasTransparentNode = false;
};
