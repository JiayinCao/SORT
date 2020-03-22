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

#include <regex>
#include "imagetexture2d.h"
#include "core/sassert.h"

#define TINYEXR_IMPLEMENTATION
#include "thirdparty/tiny_exr/tinyexr.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image/stb_image.h"

Spectrum ImageTexture2D::GetColor( int x , int y ) const{
    // if there is no image, just crash
    sAssertMsg(IS_PTR_VALID(m_memory) && IS_PTR_VALID(m_memory->m_rgb) , IMAGE , "Texture %s not loaded!" , m_name.c_str() );

    // filter the texture coordinate
    texCoordFilter( x , y );

    // get the offset
    int offset = ( m_iTexHeight - 1 - y ) * m_iTexWidth + x;

    // get the color
    return m_memory->m_rgb[ offset ];
}

float ImageTexture2D::GetAlpha( int x , int y ) const{
    // if there is no image, just crash
    sAssertMsg(IS_PTR_VALID(m_memory), IMAGE , "Texture %s not loaded!" , m_name.c_str() );

    // in case of acquiring alpha value in a texture without this channel, 1.0 is returned by default.
    if(IS_PTR_INVALID(m_memory->m_a))
        return 1.0f;

    // filter the texture coordinate
    texCoordFilter( x , y );

    // get the offset
    int offset = ( m_iTexHeight - 1 - y ) * m_iTexWidth + x;

    // get the color
    return m_memory->m_a[ offset ];
}

// load image from file
bool ImageTexture2D::LoadImageFromFile( const std::string& str ){
    static const std::regex exr_reg(".*\\.exr$", std::regex_constants::icase);

    m_memory = std::make_unique<ImgMemory>();
    m_name = str;
    if (std::regex_match(m_name, exr_reg)) {
        float* out = nullptr;
        const char* err;

        const auto ret = LoadEXR(&out, &m_iTexWidth, &m_iTexHeight, m_name.c_str(), &err);

        if (ret >= 0) {
            const auto total = m_iTexWidth * m_iTexHeight;
            m_memory->m_rgb = std::make_unique<Spectrum[]>(total);
            for (auto i = 0; i < total; i++)
                m_memory->m_rgb[i] = Spectrum(out[4 * i], out[4 * i + 1], out[4 * i + 2]);

            delete[] out;

            average();
            return true;
        }

        return false;
    }

    stbi_ldr_to_hdr_gamma(1.0f);
    stbi_ldr_to_hdr_scale(1.0f);

    auto comp = 0;
    const auto* data = stbi_loadf(m_name.c_str(), &m_iTexWidth, &m_iTexHeight, &comp, STBI_rgb_alpha);

    if (data) {
        if( m_iTexWidth > 0 && m_iTexHeight > 0 ){
            m_memory->m_rgb = std::make_unique<Spectrum[]>(m_iTexWidth*m_iTexHeight);
            for (auto i = 0; i < m_iTexHeight; ++i) {
                for (auto j = 0; j < m_iTexWidth; ++j) {
                    const auto k = i * m_iTexWidth + j;

                    auto& color = m_memory->m_rgb[i * m_iTexWidth + j];

                    color.r = data[4 * k];
                    color.g = data[4 * k + 1];
                    color.b = data[4 * k + 2];
                }
            }
        }

        // there is alpha channel in the texture.
        if( comp == STBI_rgb_alpha ){
            m_memory->m_a = std::make_unique<float[]>(m_iTexWidth*m_iTexHeight);
            for (auto i = 0; i < m_iTexHeight; ++i) {
                for (auto j = 0; j < m_iTexWidth; ++j) {
                    const auto k = i * m_iTexWidth + j;

                    auto& alpha = m_memory->m_a[i * m_iTexWidth + j];

                    alpha = data[4 * k + 3];
                }
            }
        }

        delete[] data;

        average();
        return true;
    }
    return false;
}

Spectrum ImageTexture2D::GetAverage() const{
    return m_average;
}

void ImageTexture2D::average(){
    // if there is no image, just crash
    if(IS_PTR_INVALID(m_memory) || IS_PTR_INVALID(m_memory->m_rgb))
        return;

    Spectrum average;
    for (auto i = 0; i < m_iTexHeight; ++i) {
        for (auto j = 0; j < m_iTexWidth; ++j) {
            // get the offset
            int offset = i * m_iTexWidth + j;
            // get the color
            average += m_memory->m_rgb[offset];
        }
    }

    m_average = average / (float)( m_iTexWidth * m_iTexHeight );
}
