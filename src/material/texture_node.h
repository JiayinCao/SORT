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

#pragma once

#include "material_node.h"
#include "texture/gridtexture.h"
#include "texture/checkboxtexture.h"
#include "texture/imagetexture.h"

class TextureNode : public MaterialNode{
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_COLOR; }
};

// Grid texture Node
class GridTexNode : public TextureNode
{
public:
	DEFINE_CREATOR( GridTexNode , MaterialNode , "SORTNodeGrid" );

    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
	// post process
	void PostProcess() override;

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color1" , src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color2" , src1 );

	GridTexture grid_tex;
};

// Grid texture Node
class CheckBoxTexNode : public TextureNode
{
public:
	DEFINE_CREATOR( CheckBoxTexNode , MaterialNode , "SORTNodeCheckbox" );

    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
	// post process
    void PostProcess() override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color1" , src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color2" , src1 );

	CheckBoxTexture checkbox_tex;
};

// Grid texture Node
class ImageTexNode : public TextureNode
{
public:
	DEFINE_CREATOR( ImageTexNode , MaterialNode , "SORTNodeImage" );

    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
	// post process
    void PostProcess() override;

private:
	SORT_MATERIAL_DEFINE_PROP_STR( "Filename" , filename );

	ImageTexture image_tex;
};
