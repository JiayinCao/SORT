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
#include "spectrum/spectrum.h"

// texture filter
enum TEXCOORDFILTER{
    TCF_WARP = 0 ,
    TCF_CLAMP ,
    TCF_MIRROR
};

//! @brief  Base interface for all textures in SORT.
class TextureBase {
public:
    //! @brief  Virtual destructor.
    virtual ~TextureBase() = default;

    //! @brief  Whether the texture is valid or not.
    //!
    //! @return             True if the texture is valid.
    virtual bool IsValid() const = 0;
};

//! @brief  Base class of Texture2D.
/**
 * This is still a very basic implementation of 2D texture for now. I hope I have more time to
 * implement a proper texture system sometime later.
 */
class Texture2DBase : public TextureBase {
public:
    //! @brief  Default constructor.
    Texture2DBase():m_iTexWidth(0) , m_iTexHeight(0) , m_TexCoordFilter(TEXCOORDFILTER::TCF_WARP) {}

    //! @brief  Constructor taking the size of the texture.
    Texture2DBase( int w , int h ) : m_iTexWidth(w) , m_iTexHeight(h) , m_TexCoordFilter(TEXCOORDFILTER::TCF_WARP) {}

    //! @brief  Output the texture to file
    //!
    //! @param filename     The name of the output file.
    //! @return             Whether the file is output successfully.
    bool Output( const std::string& filename );

    //! @brief  Get the color at a specific position.
    //!
    //! @param  x           X coordinate. If out of range, it will be filtered.
    //! @param  y           Y coordinate. If out of range, it will be filtered.
    //! @return             The color at the specific position.
    virtual Spectrum GetColor( int x , int y ) const = 0;

    //! @brief  Get the alpha at a specific position.
    //!
    //! @param  x           X coordinate. If out of range, it will be filtered.
    //! @param  y           Y coordinate. If out of range, it will be filtered.
    //! @return             The alpha at the specific position, it will return 1.0 for textures without alpha channel.
    virtual float GetAlpha( int x , int y ) const {
        return 1.0f;
    }

    //! @brief  Get the color given a texture coordinate.
    //!
    //! @param  u           U coordinate. If out of range, it will be filtered.
    //! @param  v           V coordinate. If out of range, it will be filtered.
    //! @return             The color at the specific texture coordinate.
    virtual Spectrum GetColorFromUV( float u , float v ) const;

    //! @brief  Get the alpha given a texture coordinate.
    //!
    //! @param  u           U coordinate. If out of range, it will be filtered.
    //! @param  v           V coordinate. If out of range, it will be filtered.
    //! @return             The alpha at the specific texture coordinate.
    virtual float GetAlphaFromtUV( float u , float v ) const;

    //! @brief  Get the width of the texture.
    //!
    //! @return             The width of the 2d texture.
    SORT_FORCEINLINE int GetWidth() const{
        return m_iTexWidth;
    }

    //! @brief  Get the height of the texture.
    //!
    //! @return             The height of the 2d texture.
    SORT_FORCEINLINE int GetHeight() const{
        return m_iTexHeight;
    }
    
    //! @brief  Whether the 2d texture is valid or not.
    //!
    //! @return             True if the texture is valid.
    bool IsValid() const override{
        return false;
    }

protected:
    /**< Size of the 2d texture. */
    int m_iTexWidth = 0;
    int m_iTexHeight = 0;

    /**< Texture filter, not configuration for now. */
    TEXCOORDFILTER  m_TexCoordFilter = TCF_WARP;

    //! @brief  Apply texture coordinate filter.
    //!
    //! @return u       U coordinate.
    //! @return v       V coordinate.
    void texCoordFilter( int& u , int&v ) const;
};

//! @brief  Base interface of 3D texture.
/**
 * This data structure could be easily extended for other shading purposes in the future.
 * However, the only purpose of texture 3D is only for volume density for now.
 */
class Texture3DBase : public TextureBase {
public:
    //! @brief  Default constructor for texture 3D.
    Texture3DBase() :m_width(0u), m_height(0u), m_depth(0u) {}

    //! @brief  Constructor taking size data.
    //!
    //! @param  w       Width of the texture.
    //! @param  h       Height of the texture.
    //! @param  d       Depth of the texture.
    Texture3DBase(unsigned w, unsigned h, unsigned d) :m_width(w), m_height(h), m_depth(d) {}

    //! @brief  Take a sample in 3D texture given a position of texel.
    //!
    //! @param  x       X coordinate position.
    //! @param  y       Y coordinate position.
    //! @param  z       Z coordinate position.
    virtual Spectrum Sample(int x, int y, int z) const = 0;

    //! @brief  Take a sample in 3D texture.
    //!
    //! @param u        U coordinate.
    //! @param v        V coordinate.
    //! @param w        W coordinate.
    virtual Spectrum Sample(float u, float v, float w) const = 0;
    
    //! @brief  Whether the 2d texture is valid or not.
    //!
    //! @return             True if the texture is valid.
    bool IsValid() const override {
        return false;
    }

protected:
    /**< Size of the 3D texture. */
    unsigned m_width;
    unsigned m_height;
    unsigned m_depth;
};