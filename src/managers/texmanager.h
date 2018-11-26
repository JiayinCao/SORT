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

#pragma once

// include header file
#include "utility/singleton.h"
#include "utility/enum.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include "spectrum/spectrum.h"

class Texture;
class ImageTexture;

class ImgMemory
{
public:
    std::unique_ptr<Spectrum[]>	m_ImgMem;
	unsigned                    m_iWidth;
	unsigned                    m_iHeight;
};

//////////////////////////////////////////////////////////////////
//	definition of texture manager
//	desc :	A textuer manager only manages image texture memory.
//			Other textures will not be managed here. And it's also
//			responsible for deallocating image memory. Two image 
//			texture will share the same image data if they are
//			loaded from the same image file.
class TexManager : public Singleton<TexManager>
{
public:
	// output the img
	// para 'str' :	name of the output entity
	// para 'tex' : texture of the output
	// para 'type': the way we output the texture
	// result     : 'true' if the texture is output successfully
	bool Write( const std::string& str , const Texture* tex );

	// load the image from file , if the specific image is already existed in the current system , just return the pointer
	// para 'str'  : name of the image file
	// para 'tex'  : output to the texture
	// result      : 'true' if loading is successful
	bool Read( const std::string& str , ImageTexture* tex );
    
private:
	// map a string to the image memory
    std::unordered_map< std::string , std::shared_ptr<ImgMemory> > m_ImgContainer;

    // para 'name'  : Load image file
    // return       : Return value
    bool loadImage(const std::string& name, std::shared_ptr<ImgMemory>& mem);
    // para 'name'  : Load image file
    // return       : Return value
    bool saveImage(const std::string& name, const Texture* mem);

	friend class Singleton<TexManager>;
};
