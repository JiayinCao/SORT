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

// include header file
#include "texmanager.h"
#include "texture/imagetexture.h"
#include "utility/strhelper.h"
#include "utility/path.h"
#include "utility/log.h"
#include <regex>

#define TINYEXR_IMPLEMENTATION
#include "thirdparty/tiny_exr/tinyexr.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image/stb_image.h"

// output texture
bool TexManager::Write( const std::string& filename , const Texture* tex )
{
	// get full path name
	std::string str = GetFullPath( filename );

    // Save image
    return saveImage(str, tex);
}

// load the image from file , if the specific image is already existed in the current system , just return the pointer
bool TexManager::Read( const std::string& str , ImageTexture* tex )
{
	// try to find the image first , if it's already existed in the system , just set a pointer
    auto it = m_ImgContainer.find( str );
	if( it != m_ImgContainer.end() )
	{
		tex->m_pMemory = it->second;
		tex->m_iTexWidth = it->second->m_iWidth;
		tex->m_iTexHeight = it->second->m_iHeight;

		return true;
	}

	// create a new memory
    std::shared_ptr<ImgMemory> mem = std::make_shared<ImgMemory>();

    if( loadImage(str, mem) ){
        // set the texture
        tex->m_pMemory = mem;
        tex->m_iTexWidth = mem->m_iWidth;
        tex->m_iTexHeight = mem->m_iHeight;

        // insert it into the container
        m_ImgContainer.insert(make_pair(str, mem));
        return true;
    }

    slog(WARNING, IMAGE, stringFormat("Can't load image file %s.", str.c_str()));
	return false;
}

// para 'name'  : Load image file
// return       : Return value
bool TexManager::loadImage(const std::string& name, std::shared_ptr<ImgMemory>& mem )
{
    std::regex exr_reg(".*\\.exr$", std::regex_constants::icase);

    if (std::regex_match(name, exr_reg)) {
        float* out = nullptr;
        int width, height;
        const char* err;

        int ret = LoadEXR(&out, &width, &height, name.c_str(), &err);

        if (ret >= 0) {
            mem->m_iWidth = width;
            mem->m_iHeight = height;
            unsigned total = mem->m_iWidth * mem->m_iHeight;
            mem->m_ImgMem = std::unique_ptr<Spectrum[]>(new Spectrum[total]);
            for (unsigned i = 0; i < total; i++)
                mem->m_ImgMem[i] = Spectrum(out[4 * i], out[4 * i + 1], out[4 * i + 2]);

            delete[] out;
            return true;
        }

        return false;
    }
    else {
        stbi_ldr_to_hdr_gamma(1.0f);
        stbi_ldr_to_hdr_scale(1.0f);

        int w, h;
        int channel = 0, desired_channel = 3;
        float* data = stbi_loadf(name.c_str(), &w, &h, &channel, desired_channel);

        int real_channel = std::min(desired_channel, channel);
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

    return true;
}

// para 'name'  : Load image file
// return       : Return value
bool TexManager::saveImage(const std::string& name, const Texture* tex )
{
    if (!tex) return false;

    std::regex exr_reg(".*\\.exr$", std::regex_constants::icase);
    if (std::regex_match(name, exr_reg)) {
        unsigned totalXRes = tex->GetWidth();
        unsigned totalYRes = tex->GetHeight();
        unsigned total = totalXRes * totalYRes;
        float *data = new float[total * 3];
        for (unsigned i = 0; i < total; ++i)
        {
            unsigned x = i % totalXRes;
            unsigned y = i / totalXRes;
            Spectrum c = tex->GetColor(x, y);

            data[3 * i] = c.GetR();
            data[3 * i + 1] = c.GetG();
            data[3 * i + 2] = c.GetB();
        }

        int ret = SaveEXR(data, tex->GetWidth(), tex->GetHeight(), 3, true, name.c_str());
        delete[] data;
        if (ret < 0)
            slog(WARNING, MATERIAL, stringFormat("Fail to save image file %s", name.c_str()));
        return ret >= 0;
    }
    
    sAssertMsg( false , IMAGE , stringFormat("SORT doesn't support exporting file %s",name.c_str()) )
    
    return false;
}
