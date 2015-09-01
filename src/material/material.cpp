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
#include "bsdf/lambert.h"
#include "bsdf/merl.h"

// default constructor
Material::Material()
{
	root = new MaterialNode();
}

Bsdf* Material::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)( intersect );
	vector<MaterialSocket>::iterator it = root->inputs.begin();
	while( it != root->inputs.end() )
	{
		it->node->UpdateBSDF(bsdf);
		++it;
	}
	return bsdf;
}

// update bsdf
void MaterialNode::UpdateBSDF( Bsdf* bsdf )
{
	vector<MaterialSocket>::iterator it = inputs.begin();
	while( it != inputs.end() )
	{
		if( it->type == Socket_Node )
			it->node->UpdateBSDF(bsdf);
		++it;
	}
}

void LambertNode::UpdateBSDF( Bsdf* bsdf )
{
	Spectrum color = inputs[0].value;
	Lambert* lambert = SORT_MALLOC(Lambert)( color );
	bsdf->AddBxdf( lambert );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf )
{
	bsdf->AddBxdf( merl );
}