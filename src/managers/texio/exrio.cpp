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
#include <ImfInputFile.h>
#include <ImathBox.h>
#include "managers/texmanager.h"
#include <half.h>
#include <ImfRgba.h>
#include <ImfRgbaFile.h>

using namespace Imf;
using namespace Imath;

// read data from file
bool ExrIO::Read( const string& name , std::shared_ptr<ImgMemory>& mem )
{
	try {
		InputFile file(name.c_str());
		Box2i dw = file.header().dataWindow();
		mem->m_iWidth  = dw.max.x - dw.min.x + 1;
		mem->m_iHeight = dw.max.y - dw.min.y + 1;

		half *rgb = new half[3 * mem->m_iWidth * mem->m_iHeight];

		FrameBuffer frameBuffer;
		frameBuffer.insert("R", Slice(HALF, (char *)rgb,
			3*sizeof(half), mem->m_iWidth * 3 * sizeof(half), 1, 1, 0.0));
		frameBuffer.insert("G", Slice(HALF, (char *)rgb+sizeof(half),
			3*sizeof(half), mem->m_iWidth * 3 * sizeof(half), 1, 1, 0.0));
		frameBuffer.insert("B", Slice(HALF, (char *)rgb+2*sizeof(half),
			3*sizeof(half), mem->m_iWidth * 3 * sizeof(half), 1, 1, 0.0));

		file.setFrameBuffer(frameBuffer);
		file.readPixels(dw.min.y, dw.max.y);

		unsigned total = mem->m_iWidth * mem->m_iHeight;
        mem->m_ImgMem = std::unique_ptr<Spectrum[]>( new Spectrum[ total ] );

		for( unsigned i = 0 ; i < total ; i++ )
			mem->m_ImgMem[i] = Spectrum( rgb[3*i] , rgb[3*i+1] , rgb[3*i+2] );

		SAFE_DELETE_ARRAY( rgb );

		return true;
    }catch (const std::exception &e) {
        LOG_WARNING<<"Unable to read image file \""<<name<<"\": "<<e.what()<<ENDL;
        return false;
    }

	return true;
}

// output the texture into bmp file
bool ExrIO::Write( const string& name , const Texture* tex )
{
	unsigned totalXRes = tex->GetWidth();
	unsigned totalYRes = tex->GetHeight();
	unsigned total = totalXRes * totalYRes;

	Rgba *hrgba = new Rgba[total];
    for (unsigned i = 0; i < total; ++i)
	{
		unsigned x = i % totalXRes;
		unsigned y = i / totalXRes;
		Spectrum c = tex->GetColor( x , y );
		hrgba[i] = Rgba( c.GetR() , c.GetG() , c.GetB() , 1.f);
	}
	
    Box2i displayWindow(V2i(0,0), V2i(totalXRes-1, totalYRes-1));
    Box2i dataWindow(V2i(0, 0), V2i(totalXRes - 1, totalYRes - 1));

    try {
        RgbaOutputFile file(name.c_str(), displayWindow, dataWindow, WRITE_RGBA);
        file.setFrameBuffer(hrgba, 1, totalXRes);
        file.writePixels(totalYRes);
    }
    catch (const std::exception &e) {
		LOG_WARNING<<"Unable to write image file \""<<name<<"\": "<<e.what()<<ENDL;
    }

    delete[] hrgba;

	return true;
}
