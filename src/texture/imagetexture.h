/*
 * filename :	imagetexture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_IMAGETEXTURE
#define	SORT_IMAGETEXTURE

// include the header file
#include "texture.h"

// pre-declare memory
struct ImgMemory;

///////////////////////////////////////////////////////////////
// defination of image texture
class ImageTexture : public Texture
{
// public method
public:
	// default constructor
	ImageTexture() { _init(); }
	// destructor
	~ImageTexture() { Release(); }

	// load image from file
	// para 'str'  : the name of the image file to be loaded
	// para 'type' : the type of the image , default is bmp
	// result      : whether loading is successful
	bool LoadImage( const std::string& str , TEX_TYPE type = TT_BMP );

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const;

	// it means nothing to set the size of the image file
	// set the size of the texture
	virtual void	SetSize( unsigned w , unsigned h ){}

	// release the texture memory
	virtual void Release();

// private field
private:
	// array saving the color of image
	ImgMemory*	m_pMemory;

// private method
	// initialize default data
	void	_init();

	// set texture manager as a friend
	friend class TexManager;
};

#endif
