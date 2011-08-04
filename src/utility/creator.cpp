/*
   FileName:      creator.cpp

   Created Time:  2011-08-04 12:42:14

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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
#include "light/spot.h"
#include "light/distant.h"

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
	REGISTER( "spot" , SpotLight );
	REGISTER( "distant" , DistantLight );

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
