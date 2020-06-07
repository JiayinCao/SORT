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

#include "texture_system.h"
#include "core/log.h"
#include "core/mesh.h"

// static const OSL::ustring s_volume_color("volume_color");
// static const OSL::ustring s_volume_density("volume_density");

// RendererServices::TextureHandle* SORTTextureSystem::get_texture_handle(OSL::ustring filename, Perthread *thread_info) {
//     // handle pre-defined 3d textures
//     if (s_volume_color == filename || s_volume_density == filename)
//         return nullptr;

//     // Only process the texture if it has been seen before.
//     std::string std_filename(filename.c_str());
//     if (m_Texture2DPool.count(std_filename) == 0) {
//         // create a new texture and load it from file
//         m_Texture2DPool[std_filename] = std::make_unique<ImageTexture2D>();

//         // load the texture from file
//         ImageTexture2D*   sort_texture = m_Texture2DPool[std_filename].get();
//         if( !sort_texture->LoadImageFromFile(std_filename) )
//             slog( WARNING , MATERIAL , "Failed to load image %s." , filename.c_str() );
//     }

//     return (RendererServices::TextureHandle*)m_Texture2DPool[std_filename].get();
// }

// bool SORTTextureSystem::good(TextureHandle* texture_handle) {
//     auto image_texture = reinterpret_cast<const TextureBase*>(texture_handle);
//     return IS_PTR_VALID(image_texture) && image_texture->IsValid();
// }

// bool SORTTextureSystem::texture(TextureHandle *texture_handle, Perthread *thread_info, TextureOpt &options,
//     float s, float t, float dsdx, float dtdx, float dsdy, float dtdy, int nchannels, float *result,
//     float *dresultds , float *dresultdt ) {
//     auto image_texture = reinterpret_cast<const ImageTexture2D*>(texture_handle);
//     const auto color = image_texture->GetColorFromUV(s, t);
//     result[0] = color.r;
//     result[1] = color.g;
//     result[2] = color.b;

//     if( nchannels > 3 )
//         result[3] = image_texture->GetAlphaFromtUV(s, t);

//     return true;
// }

// bool SORTTextureSystem::texture3d(ustring filename, TextureOpt& options, const Imath::V3f& P, const Imath::V3f& dPdx,
//     const Imath::V3f& dPdy, const Imath::V3f& dPdz, int nchannels, float* result, float* dresultds , 
//     float* dresultdt , float* dresultdr ) {
//     const auto* thread_info = reinterpret_cast<const SORTTextureThreadInfo*>(get_perthread_info());

//     // make sure there is valid mesh, otherwise, just return 0.0f.
//     const auto* mesh = thread_info->mesh;
//     if (IS_PTR_INVALID(mesh)) {
//         if (s_volume_color == filename) {
//             result[0] = result[1] = result[2] = 0.0f;
//             return true;
//         }
//         else if (s_volume_density == filename) {
//             result[0] = 0.0f;
//             return true;
//         }
//     }

//     if (s_volume_color == filename) {
//         const auto color = mesh->SampleVolumeColor(Point(P));
//         result[0] = color[0];
//         result[1] = color[1];
//         result[2] = color[2];
//         return true;
//     } else if (s_volume_density == filename) {
//         result[0] = mesh->SampleVolumeDensity(Point(P));
//         return true;
//     }

//     return false;
// }

// TextureSystem::Perthread* SORTTextureSystem::get_perthread_info(TextureSystem::Perthread* thread_info){
//     static thread_local SORTTextureThreadInfo s_thread_info;
//     return (TextureSystem::Perthread*)&s_thread_info;
// }

// TextureSystem::Perthread* SORTTextureSystem::create_thread_info() {
//     return nullptr;
// }

// void SORTTextureSystem::destroy_thread_info(TextureSystem::Perthread* threadinfo){
// }