/*
   FileName:      material.h

   Created Time:  2015-09-01 20:23:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "material.h"
#include "bsdf/bsdf.h"
#include "managers/memmanager.h"

// default constructor
Material::Material()
{
	root = new OutputNode();
}

Bsdf* Material::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)( intersect );
	root->UpdateBSDF(bsdf);
	return bsdf;
}

// parse material
void Material::ParseMaterial( TiXmlElement* element )
{
	// parse node property
	root->ParseProperty( element , root );

	// post process material
	root->PostProcess();
}
