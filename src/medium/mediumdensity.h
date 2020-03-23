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
#include "texture/imagetexture3d.h"

struct Point;

//! @brief  Medium density data structure allows variation of density inside a medium volume.
/**
 * Medium density is essentially a 3D texture.
 */
class MediumDensity : public ImageTexture3D {
public:
    //! @brief  Take a sample in 3D texture.
    //!
    //! @param  pos     Position in world space.
    Spectrum Sample( const Point& pos ) const;

    //! @brief  Whether the 2d texture is valid or not.
    //!
    //! @return             True if the texture is valid.
    bool IsValid() const override {
        return true;
    }
};

//! @brief  Medium color data structure allows variation of color inside a medium volume.
class MediumColor : public ImageTexture3D {
public:
    //! @brief  Take a sample in 3D texture.
    //!
    //! @param  pos     Position in world space.
    Spectrum Sample(const Point& pos) const;

    //! @brief  Whether the 2d texture is valid or not.
    //!
    //! @return             True if the texture is valid.
    bool IsValid() const override {
        return true;
    }
};