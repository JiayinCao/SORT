/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_IMAGETEXTURE
#define	SORT_IMAGETEXTURE

// include the header file
#include "texture.h"
#include "utility/referencecount.h"
#include "managers/texmanager.h"

///////////////////////////////////////////////////////////////
// definition of image texture
class ImageTexture : public Texture 
{
// public method
public:
	DEFINE_CREATOR( ImageTexture , "image" );

	// default constructor
	ImageTexture() { _init(); }
	// destructor
	~ImageTexture() { Release(); }

	// load image from file
	// para 'str'  : the name of the image file to be loaded
	// result      : whether loading is successful
	bool LoadImageFromFile( const std::string& str );

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const;

	// get texture value
	// para 'u' : u coordinate
	// para 'v' : v coordinate
	// result   : spectrum value
	virtual Spectrum GetColor( float u , float v ) const
	{
		int w = (int) (u * ( m_iTexWidth - 1 ) + 0.5f );
		int h = (int) (v * ( m_iTexHeight - 1 ) + 0.5f );
		return GetColor( w , h );
	}

	// it means nothing to set the size of the image file
	// set the size of the texture
	virtual void	SetSize( unsigned w , unsigned h ){}

	// release the texture memory
	virtual void Release();

	// whether the image is valid
	bool IsValid() { return (bool)m_pMemory; }

	// get average color
	Spectrum GetAverage() const;

// private field
private:
	// array saving the color of image
	Reference<ImgMemory>	m_pMemory;

	// the average radiance of the texture
	Spectrum	m_Average;

// private method
	// initialize default data
	void	_init();
	// compute average radiance
	void	_average();

	// set texture manager as a friend
	friend class TexManager;

	// register properties
	void _registerAllProperty();

// property handler
	class FileNameProperty : public PropertyHandler<Texture>
	{
	public:
		PH_CONSTRUCTOR(FileNameProperty,Texture);

		// set value
		void SetValue( const string& value )
		{
			ImageTexture* ct = CAST_TARGET(ImageTexture);
			
			// load image file
			ct->LoadImageFromFile( value );
		}
	};
};

#endif
