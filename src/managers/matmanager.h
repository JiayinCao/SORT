/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

#include "sort.h"
#include <vector>
#include <memory>
#include "core/singleton.h"

/////////////////////////////////////////////////////////////////////////////
//	definition of material manager
//	desc :	Material manager is a singleton. All of the materials in the system
//			are parsed from file and located in a single pool. And it's 
//			responsible for deallocate all of the material memory.
//	note :	There will be textures in most of materials. when materials are
//			deleted the bind texture is also deleted. In another words ,
//			material system is also responsible for deallocating textures.
class	MatManager : public Singleton<MatManager>
{
public:
    //! @brief  Find material by id.
    //!
    //! Materials are packed in a specific order that matches with the ids in index buffer.
    //!
    //! @param  matId   Id of the material. If this is an out-of-boundary id, default material
    //!                 will be returned.
    //! @return         Shared pointer for material returned.
    std::shared_ptr<class Material> GetMaterial(int matId) const {
        if (matId < 0 || matId >(int)m_matPool.size())
            return GetDefaultMat();
        return m_matPool[matId];
    }

	// get default material
    std::shared_ptr<class Material>	GetDefaultMat() const{
        static std::shared_ptr<Material> defaultMat = std::make_shared<Material>();
        return defaultMat;
    }

	// parse material file and add the materials into the manager
	// result           : the number of materials in the file
	unsigned	ParseMatFile( class IStreamBase& stream );

private:
    std::vector<std::shared_ptr<class Material>> m_matPool;     /**< Material pool holding all materials. */

	friend class Singleton<MatManager>;
};
