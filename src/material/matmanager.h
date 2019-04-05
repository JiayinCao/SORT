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
#include <memory>
#include <unordered_map>
#include "core/singleton.h"
#include "material/material.h"
#include "resource.h"

//! @brief Material manager.
/**
 * This could very likely be a temporary solution for now.
 */
class	MatManager : public Singleton<MatManager>
{
public:
    //! @brief  Find material by id.
    //!
    //! Materials are packed in a specific order that matches with the ids in index buffer.
    //!
    //! @param  matId   Id of the material. If this is an out-of-boundary id, default material
    //!                 will be returned.
    //! @return         Pointer for material returned.
    class Material* GetMaterial(int matId) const {
        if (matId < 0 || matId >= (int)m_matPool.size())
            return GetDefaultMat();
        return m_matPool[matId].get();
    }

	// get default material
    class Material*	GetDefaultMat() const{
        static Material defaultMat;
        return &defaultMat;
    }

	// parse material file and add the materials into the manager
	// result           : the number of materials in the file
	unsigned	ParseMatFile( class IStreamBase& stream );

    //! @brief  Construct shader source code given a list of parameters in string format.
    //!
    //! @param  shaderName  The name of the shader to be constructed.
    //! @param  shaderType  The type of shader to be constructed.
    //! @param  paramValue  The default value of the parameters in string format to be set.
    //! @return             The constructed shader source code.
    std::string ConstructShader(const std::string& shaderName, const std::string& shaderType, const std::vector<std::string>& paramValue);

    //! @brief  Get resource data based on index.
    //!
    //! @param  index       Index of the resource.
    //! @return             The pointer of the resource. 'nullptr' will be returned if the index is out of range.
    Resource*   GetResource(int index);

private:
    std::vector<std::unique_ptr<class Material>>     m_matPool;         /**< Material pool holding all materials. */
    std::unordered_map<std::string, std::string>     m_shaderSources;   /**< OSL shader source code. */

    std::vector<std::unique_ptr<Resource>>           m_resources;       /**< Resources used during BXDF evaluation. */

	friend class Singleton<MatManager>;
};
