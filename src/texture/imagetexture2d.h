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

#include <memory>
#include "texturebase.h"

//! @brief  Image texture.
/**
 * Image texture is the most commonly used texture. It is just a two dimensional set of pixels.
 * There is no mip-map solution for now.
 */
class ImageTexture2D : public Texture2DBase{
public:
    //! @brief  Load an image from file.
    //!
    //! @param  filename    The name of the file to be loaded.
    //! @return             Whether the file is loaded correctly.
    bool LoadImageFromFile( const std::string& filename );

    //! @brief  Get the color at a specific position.
    //!
    //! @param  x           X coordinate. If out of range, it will be filtered.
    //! @param  y           Y coordinate. If out of range, it will be filtered.
    //! @return             The color at the specific position.
    Spectrum GetColor( int x , int y ) const override;

    //! @brief  Get the alpha at a specific position.
    //!
    //! @param  x           X coordinate. If out of range, it will be filtered.
    //! @param  y           Y coordinate. If out of range, it will be filtered.
    //! @return             The alpha at the specific position, it will return 1.0 for textures without alpha channel.
    float GetAlpha( int x , int y ) const override;

    //! @brief  Whether the 2d texture is valid or not.
    //!
    //! @return             True if the texture is valid.
    bool IsValid() const override { 
        return IS_PTR_VALID(m_memory); 
    }

    //! @brief  Get the average color of the texture.
    //!
    //! @return             The average color of the texture.
    Spectrum GetAverage() const;

private:
    class ImgMemory{
    public:
        std::unique_ptr<Spectrum[]>     m_rgb = nullptr;   /**< RGB Channels. */
        std::unique_ptr<float[]>        m_a  = nullptr;   /**< Alpha Channel. */
    };

    // array saving the color of image
    std::unique_ptr<ImgMemory>  m_memory = nullptr;

    // the average radiance of the texture
    Spectrum    m_average;

    // texture name
    std::string m_name;

    // compute average radiance
    void    average();
};
