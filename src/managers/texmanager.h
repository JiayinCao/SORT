/*
 * filename :	texmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TEXMANAGER
#define	SORT_TEXMANAGER

// include header file
#include "utility/singleton.h"
#include "utility/enum.h"
#include <vector>
#include <map>
#include "spectrum/spectrum.h"
#include "utility/referencecount.h"

class Texture;
class ImageTexture;
class TexIO;

class ImgMemory : public ReferenceCount
{
public:
	Spectrum*	m_ImgMem;
	unsigned	m_iWidth;
	unsigned	m_iHeight;
};

//////////////////////////////////////////////////////////////////
//	definition of texture manager
class TexManager : public Singleton<TexManager>
{
// public method
public:
	// destructor
	~TexManager();

	// output the img
	// para 'str' :	name of the output entity
	// para 'tex' : texture of the output
	// para 'type': the way we output the texture
	// result     : 'true' if the texture is output successfully
	bool Write( const string& str , const Texture* tex );

	// load the image from file , if the specific image is already existed in the current system , just return the pointer
	// para 'str'  : name of the image file
	// para 'tex'  : output to the texture
	// result      : 'true' if loading is successful
	bool Read( const string& str , ImageTexture* tex );

	// get the reference count
	// para 'str' : the name of the image file
	// result     : how many reference to the memory
	unsigned GetReferenceCount( const string& str ) const;

	// create a texture
	Texture* CreateTexture( const string& type ) const;	

// private data
private:
	// a vector saving texture io
	vector<TexIO*>	m_TexIOVec;

	// map a string to the image memory
	map< string , ImgMemory* > m_ImgContainer;
	
	// texture map
	map< string , Texture* > m_TextureMap;

// private method
private:
	// private default constructor
	TexManager();

	// initialize texture manager data
	void _init();

	// release texture manager data
	void _release();

	// register all texture types
	void _registerTexture();

	// unregister texture
	void _unregisterTexture();

	// find correct texio
	TexIO*	FindTexIO( TEX_TYPE tt ) const;

	friend class Singleton<TexManager>;
};

#endif
