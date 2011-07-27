/*
 * filename	:	creator.cpp
 *
 * programmer :	Cao Jiayin
 */

#include "creator.h"

// the materials
#include "material/mirror.h"
#include "material/matte.h"
#include "material/merlmat.h"
#include "material/glass.h"

// the textures
#include "texture/constanttexture.h"
#include "texture/gridtexture.h"
#include "texture/checkboxtexture.h"
#include "texture/imagetexture.h"
#include "texture/normaltexture.h"
#include "texture/uvtexture.h"

// acceleration structure
#include "accel/kdtree.h"
#include "accel/bvh.h"
#include "accel/unigrid.h"

// lights
#include "light/pointlight.h"

// instance the singleton with logmanager class
DEFINE_SINGLETON(Creator);

// initialize the system
void Creator::_init()
{
#define	REGISTER( str , T ) m_container.insert( make_pair( str , new T::T##Creator() ) );

	//----------------------------------------------------------
	// materials
	REGISTER( "Mirror" , Mirror );
	REGISTER( "Glass" , Glass );
	REGISTER( "Merl" , MerlMat );
	REGISTER( "Matte" , Matte );

	//----------------------------------------------------------
	// textures
	REGISTER( "constant" , ConstantTexture );
	REGISTER( "checkbox" , CheckBoxTexture );
	REGISTER( "grid" , GridTexture );
	REGISTER( "image" , ImageTexture );
	REGISTER( "normal" , NormalTexture );
	REGISTER( "uv" , UVTexture );

	//----------------------------------------------------------
	// acceleration structure
	REGISTER( "kd_tree" , KDTree );
	REGISTER( "bvh" , Bvh );
	REGISTER( "uniform_grid" , UniGrid );

	//----------------------------------------------------------
	// lights
	REGISTER( "point" , PointLight );

#undef	REGISTER
}

// release the system
void Creator::_release()
{
	map<string,ItemCreator*>::iterator it = m_container.begin();
	while( it != m_container.end() )
	{
		delete it->second;
		it++;
	}
	m_container.clear();
}