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

//! @brief Resource
/**
 * A resource is used in material. It could be a texture, measured BRDF, or anything that is not suitable loaded during evaluation of shader,
 * but it is still needed during the process.
 */
class Resource {
public:
    //! @brief  Default virtual destructor.
    virtual ~Resource() = default;

    //! @brief  Load the resource from file.
    //!
    //! @param  filename        Name of the external file holding the data.
    //! @return                 Whether the file has been loaded successfully.
    virtual bool LoadResource(const std::string& filename) = 0;
};