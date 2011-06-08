/*
 * filename :	texmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_TEXMANAGER
#define	SORT_TEXMANAGER

// include header file
#include "../../sort.h"
#include "../../utility/singleton.h"
#include "texio/texio.h"
#include "../../texture/texture.h"
#include <vector>

//////////////////////////////////////////////////////////////////
//	defination of texture manager
class TexManager : public Singleton<TexManager>
{
// public method
public:
	// create TexManager
	static void CreateTexManager();

	// output the img
	// para 'str' :	name of the output entity
	// para 'tex' : texture of the output
	// para 'type': the way we output the texture
	// result     : 'true' if the texture is output successfully
	bool Write( const string& str , const Texture* tex , TEX_OUTPUT_TYPE type );

// private data
private:
	// a vector saving texture io
	vector<TexIO*>	m_TexIOVec;

// private method
private:
	// private default constructor
	TexManager();

	// initialize texture manager data
	void _init();
};

#endif
