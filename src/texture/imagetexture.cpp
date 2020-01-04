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
#include "imagetexture.h"
#include "core/sassert.h"

#define TINYEXR_IMPLEMENTATION
#include "thirdparty/tiny_exr/tinyexr.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image/stb_image.h"

// get color from image
Spectrum ImageTexture::GetColor( int x , int y ) const{
    // if there is no image, just crash
    sAssertMsg( m_pMemory != 0 && m_pMemory->m_ImgMem != 0 , IMAGE , "Texture %s not loaded!" , m_Name.c_str() );

    // filter the texture coordinate
    _texCoordFilter( x , y );

    // get the offset
    int offset = ( m_iTexHeight - 1 - y ) * m_iTexWidth + x;

    // get the color
    return m_pMemory->m_ImgMem[ offset ];
}

float ImageTexture::GetAlpha( int x , int y ) const{
    // if there is no image, just crash
    sAssertMsg( m_pMemory != 0 , IMAGE , "Texture %s not loaded!" , m_Name.c_str() );

    // in case of acquiring alpha value in a texture without this channel, 1.0 is returned by default.
    if( nullptr == m_pMemory->m_Alpha )
        return 1.0f;

    // filter the texture coordinate
    _texCoordFilter( x , y );

    // get the offset
    int offset = ( m_iTexHeight - 1 - y ) * m_iTexWidth + x;

    // get the color
    return m_pMemory->m_Alpha[ offset ];
}

// load image from file
bool ImageTexture::LoadImageFromFile( const std::string& str ){
    static const std::regex exr_reg(".*\\.exr$", std::regex_constants::icase);

    m_pMemory = std::make_unique<ImgMemory>();
    m_Name = str;
    if (std::regex_match(m_Name, exr_reg)) {
        float* out = nullptr;
        const char* err;

        const auto ret = LoadEXR(&out, &m_iTexWidth, &m_iTexHeight, m_Name.c_str(), &err);

        if (ret >= 0) {
            const auto total = m_iTexWidth * m_iTexHeight;
            m_pMemory->m_ImgMem = std::make_unique<Spectrum[]>(total);
            for (auto i = 0; i < total; i++)
                m_pMemory->m_ImgMem[i] = Spectrum(out[4 * i], out[4 * i + 1], out[4 * i + 2]);

            delete[] out;

            _average();
            return true;
        }

        return false;
    }

    stbi_ldr_to_hdr_gamma(1.0f);
    stbi_ldr_to_hdr_scale(1.0f);

    auto comp = 0;
    float* data = stbi_loadf(m_Name.c_str(), &m_iTexWidth, &m_iTexHeight, &comp, STBI_rgb_alpha);

    if (data) {
        if( comp == STBI_rgb || comp == STBI_rgb_alpha ){
            m_pMemory->m_ImgMem = std::make_unique<Spectrum[]>(m_iTexWidth*m_iTexHeight);
            for (auto i = 0; i < m_iTexHeight; ++i) {
                for (auto j = 0; j < m_iTexWidth; ++j) {
                    const auto k = i * m_iTexWidth + j;

                    auto& color = m_pMemory->m_ImgMem[i * m_iTexWidth + j];

                    color.r = data[4 * k];
                    color.g = data[4 * k + 1];
                    color.b = data[4 * k + 2];
                }
            }
        }

        // there is alpha channel in the texture.
        if( comp == STBI_rgb_alpha ){
            m_pMemory->m_Alpha = std::make_unique<float[]>(m_iTexWidth*m_iTexHeight);
            for (auto i = 0; i < m_iTexHeight; ++i) {
                for (auto j = 0; j < m_iTexWidth; ++j) {
                    const auto k = i * m_iTexWidth + j;

                    auto& alpha = m_pMemory->m_Alpha[i * m_iTexWidth + j];

                    alpha = data[4 * k + 3];
                }
            }
        }

        delete[] data;

        _average();
        return true;
    }
    return false;
}

// get average color
Spectrum ImageTexture::GetAverage() const{
    return m_Average;
}

// compute average radiance
void ImageTexture::_average(){
    // if there is no image, just crash
    if( m_pMemory == nullptr || m_pMemory->m_ImgMem == nullptr )
        return;

    Spectrum average;
    for (auto i = 0; i < m_iTexHeight; ++i) {
        for (auto j = 0; j < m_iTexWidth; ++j) {
            // get the offset
            int offset = i * m_iTexWidth + j;
            // get the color
            average += m_pMemory->m_ImgMem[offset];
        }
    }

    m_Average = average / (float)( m_iTexWidth * m_iTexHeight );
}
