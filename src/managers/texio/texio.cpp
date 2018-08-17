/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "texio.h"
#include "managers/texmanager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image/stb_image.h"

bool TexIO::Read(const string& str, std::shared_ptr<ImgMemory>& mem)
{
    stbi_ldr_to_hdr_gamma(1.0f);
    stbi_ldr_to_hdr_scale(1.0f);

    int w, h;
    int channel = 0, desired_channel = 3;
    float* data = stbi_loadf(str.c_str(), &w, &h, &channel, desired_channel);

    int real_channel = min(desired_channel, channel);
    if (data && real_channel) {
        mem->m_ImgMem = std::unique_ptr<Spectrum[]>(new Spectrum[w*h]);
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                int k = i * w + j;
                float r = data[real_channel * k];
                float g = data[real_channel * k + 1];
                float b = data[real_channel * k + 2];
                mem->m_ImgMem[i * w + j].SetColor(r, g, b);
            }
        }
        mem->m_iWidth = w;
        mem->m_iHeight = h;

        delete[] data;
        return true;
    }
    return false;
}