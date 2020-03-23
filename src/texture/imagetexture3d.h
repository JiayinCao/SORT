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

#include "texturebase.h"

//! @brief  3D image texture.
/**
 * 3D image texture is a three dimentional set of pixel data.
 */
class ImageTexture3D : public Texture3DBase{
public:
    //! @brief  Default constructor.
    ImageTexture3D() : Texture3DBase(0u, 0u, 0u) {}

    //! @brief  Constructor taking size data.
    //!
    //! @param  w       Width of the texture.
    //! @param  h       Height of the texture.
    //! @param  d       Depth of the texture.
    //! @param  data    Memory data of the texture 3D.
    ImageTexture3D(unsigned w, unsigned h, unsigned d, const float* data);

    //! @brief  Take a sample in 3D texture given a position of texel.
    //!
    //! @param  x       X coordinate position.
    //! @param  y       Y coordinate position.
    //! @param  z       Z coordinate position.
    Spectrum Sample(int x, int y, int z) const override;

    //! @brief  Take a sample in 3D texture.
    //!
    //! @param u        U coordinate.
    //! @param v        V coordinate.
    //! @param w        W coordinate.
    Spectrum Sample(float u, float v, float w) const override;

protected:
    class ImgMemory {
    public:
        std::unique_ptr<Spectrum[]>     m_rgb = nullptr;   /**< RGB Channels. */
    };

    /**< 3d texture memory. */
    std::unique_ptr<ImgMemory>  m_memory = nullptr;
};