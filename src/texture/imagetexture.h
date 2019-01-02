/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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

#include "texture.h"
#include "managers/texmanager.h"

///////////////////////////////////////////////////////////////
// definition of image texture
class ImageTexture : public Texture 
{
public:
	DEFINE_CREATOR( ImageTexture , Texture , "image" );

	// load image from file
	// para 'str'  : the name of the image file to be loaded
	// result      : whether loading is successful
	bool LoadImageFromFile( const std::string& str );

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
    Spectrum GetColor( int x , int y ) const override;

	// it means nothing to set the size of the image file
	// set the size of the texture
	void	SetSize( unsigned w , unsigned h ) override{}

	// whether the image is valid
	bool IsValid() override { return (bool)m_pMemory; }

	// get average color
	Spectrum GetAverage() const;

private:
	// array saving the color of image
    std::shared_ptr<ImgMemory>	m_pMemory = nullptr;

	// the average radiance of the texture
	Spectrum	m_Average;

    // texture name
    std::string m_Name;

	// compute average radiance
	void	_average();

	// set texture manager as a friend
	friend class TexManager;
};
