/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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
class IStreamBase;

//! @brief  Medium density data structure allows variation of density inside a medium volume.
/**
 * Medium density is essentially a 3D texture.
 */
class MediumDensity : public ImageTexture3D<float> {
public:
    //! @brief  Take a sample in 3D texture.
    //!
    //! @param  uvw     Texture coordinate in volume space.
    //! @return         The density at the position related to the object.
    float   Sample( const Point& uvw) const;

    //! @brief      Serializing data from stream.
    //!
    //! @param  Stream  where the serialization data comes from. Depending on different situation,
    //!                 it could come from different places.
    void    Serialize(IStreamBase& stream);
};

//! @brief  Medium color data structure allows variation of color inside a medium volume.
class MediumColor : public ImageTexture3D<Spectrum> {
public:
    //! @brief  Take a sample in 3D texture.
    //!
    //! @param  uvw     Texture coordinate in volume space.
    //! @return         The color at the position related to the object.
    Spectrum Sample(const Point& uvw) const;

    //! @brief      Serializing data from stream.
    //!
    //! @param  Stream  where the serialization data comes from. Depending on different situation,
    //!                 it could come from different places.
    void    Serialize(IStreamBase& stream);
};