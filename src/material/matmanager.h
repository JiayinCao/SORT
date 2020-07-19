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
#include <memory>
#include <unordered_map>
#include "core/singleton.h"
#include "material/material.h"
#include "core/resource.h"
#include "task/task.h"

//! @brief Material manager.
/**
 * This could very likely be a temporary solution for now.
 */
class   MatManager : public Singleton<MatManager>{
public:
    //! @brief  Find material by id.
    //!
    //! Materials are packed in a specific order that matches with the ids in index buffer.
    //!
    //! @param  matId   Id of the material. If this is an out-of-boundary id, default material
    //!                 will be returned.
    //! @return         Pointer for material returned.
    const MaterialBase* GetMaterial(int matId) const {
        if (matId < 0 || matId >= (int)m_matPool.size())
            return GetDefaultMat();
        return m_matPool[matId].get();
    }

    //! @brief  Create a material proxy given a material.
    //!
    //! @param  material    The material to be proxied.
    //! @return             A material proxy that refers the to provided material.
    const MaterialBase* CreateMaterialProxy(const MaterialBase& material);

    // get default material
    const MaterialBase* GetDefaultMat() const{
        static Material defaultMat;
        return &defaultMat;
    }

    // parse material file and add the materials into the manager
    // result           : the number of materials in the file
    unsigned    ParseMatFile( class IStreamBase& stream );

    //! @brief  Get resource data based on index.
    //!
    //! @param  name        Name of the resource.
    //! @return             The pointer of the resource. 'nullptr' will be returned if the index is out of range.
    const Resource*   GetResource(const std::string& name) const;

    //! @brief  Retrieve shader units through shader unit template type.
    //!
    //! @param  name_id     The name of the template.
    //! @return             The shader unit template returned, nullptr if it doesn't exist.
    std::shared_ptr<ShaderUnitTemplate> GetShaderUnitTemplate(const std::string& name) const;

#ifdef ENABLE_MULTI_THREAD_SHADER_COMPILATION
    //! @brief  Wait for all materials to be built before moving forward
    void WaitForMaterialBuilding() const;
#endif

private:
    std::vector<std::unique_ptr<MaterialBase>>       m_matPool;         /**< Material pool holding all materials. */

    std::unordered_map<std::string, std::unique_ptr<Resource>>  m_resources;       /**< Resources used during BXDF evaluation. */

    std::unordered_map<std::string, std::shared_ptr<ShaderUnitTemplate>>     m_shader_units;

    /**< Shader unit default values. */
    std::vector<ShaderParamDefaultValue>        m_paramDefaultValues;

    friend class Singleton<MatManager>;
};
