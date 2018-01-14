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
#include "texio/bmpio.h"
#include "texio/exrio.h"
#include "texio/tgaio.h"
#include "texio/pngio.h"
#include "texio/jpgio.h"
#include "texio/hdrio.h"
#include "texture/imagetexture.h"
#include "utility/strhelper.h"
#include "utility/define.h"
#include "utility/path.h"
#include "texture/checkboxtexture.h"
#include "texture/gridtexture.h"
#include "texture/constanttexture.h"
#include "texture/imagetexture.h"
#include "log/log.h"

// default constructor
TexManager::TexManager()
{
    // push the texture outputer
    m_TexIOVec.push_back( std::unique_ptr<TexIO>(new BmpIO()) );
    m_TexIOVec.push_back( std::unique_ptr<TexIO>(new ExrIO()) );
    m_TexIOVec.push_back( std::unique_ptr<TexIO>(new TgaIO()) );
    m_TexIOVec.push_back( std::unique_ptr<TexIO>(new PngIO()) );
    m_TexIOVec.push_back( std::unique_ptr<TexIO>(new JpgIO()) );
    m_TexIOVec.push_back( std::unique_ptr<TexIO>(new HdrIO()) );
}

// output texture
bool TexManager::Write( const string& filename , const Texture* tex )
{
	// get full path name
	string str = GetFullPath( filename );

	// get the type
	TEX_TYPE type = TexTypeFromStr( str );

	// find the specific texio first
    const std::unique_ptr<TexIO>& io = FindTexIO( type );

	if( io != nullptr )
		io->Write( str , tex );
	
	return true;
}

// load the image from file , if the specific image is already existed in the current system , just return the pointer
bool TexManager::Read( const string& filename , ImageTexture* tex )
{
	// get full path name
	string str = filename;

	// get the type
	TEX_TYPE type = TexTypeFromStr( str );

	// try to find the image first , if it's already existed in the system , just set a pointer
    auto it = m_ImgContainer.find( str );
	if( it != m_ImgContainer.end() )
	{
		tex->m_pMemory = it->second;
		tex->m_iTexWidth = it->second->m_iWidth;
		tex->m_iTexHeight = it->second->m_iHeight;

		return true;
	}

	// find the specific texio first
    const std::unique_ptr<TexIO>& io = FindTexIO( type );

	bool read = false;
	if( io != nullptr )
	{
		// create a new memory
        std::shared_ptr<ImgMemory> mem = std::make_shared<ImgMemory>();

		// read the data
		read = io->Read( str , mem );

		if( read )
		{
			// set the texture
			tex->m_pMemory = mem;
			tex->m_iTexWidth = mem->m_iWidth;
			tex->m_iTexHeight = mem->m_iHeight;
			
			// insert it into the container
			m_ImgContainer.insert( make_pair( str , mem ) );
		}else
            slog( LOG_LEVEL::WARNING , LOG_TYPE::IMAGE , stringFormat("Can't load image file Ts." , str.c_str() ) );
	}

	return read;
}

// find correct texio
const std::unique_ptr<TexIO>& TexManager::FindTexIO( TEX_TYPE tt ) const
{
	// find the specific texio first
    auto it = m_TexIOVec.begin();
	while( it != m_TexIOVec.end() )
	{
		if( (*it)->GetTT() == tt )
            return *it;
		it++;
	}

	return m_TexNull;
}
