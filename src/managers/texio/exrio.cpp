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

// include the header
#include "exrio.h"
#include "texture/texture.h"
#include "managers/texmanager.h"
#include "utility/log.h"


#include "thirdparty/tiny_exr/tinyexr.h"

// read data from file
bool ExrIO::Read( const string& name , std::shared_ptr<ImgMemory>& mem )
{
    float* out = nullptr;
    int width , height;
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

    slog( WARNING , MATERIAL , err );
	return false;
}

// output the texture into bmp file
bool ExrIO::Write( const string& name , const Texture* tex )
{
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
