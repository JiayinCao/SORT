/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "texture_system.h"

RendererServices::TextureHandle* SORTTextureSystem::get_texture_handle(OSL::ustring filename, Perthread *thread_info) {
    // Only process the texture if it has been seen before.
    std::string std_filename(filename.c_str());
    if (m_TexturePool.count(std_filename) == 0) {
        // create a new texture and load it from file
        m_TexturePool[std_filename] = std::make_unique<ImageTexture>();

        // load the texture from file
        ImageTexture*   sort_texture = m_TexturePool[std_filename].get();
        sort_texture->LoadImageFromFile(std_filename);
    }

    return (RendererServices::TextureHandle*)m_TexturePool[std_filename].get();
}

bool SORTTextureSystem::good(TextureHandle* texture_handle) {
    auto image_texture = (ImageTexture*)texture_handle;
    return image_texture && image_texture->IsValid();
}

bool SORTTextureSystem::texture(TextureHandle *texture_handle, Perthread *thread_info, TextureOpt &options,
    float s, float t, float dsdx, float dtdx, float dsdy, float dtdy, int nchannels, float *result,
    float *dresultds , float *dresultdt ) {
    const auto* image_texture = (const ImageTexture*)texture_handle;
    RGBSpectrum color = image_texture->GetColorFromUV(s, t);
    result[0] = color.r;
    result[1] = color.g;
    result[2] = color.b;
    return true;
}