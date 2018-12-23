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

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        src0.Serialize( stream );
        src1.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );

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
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        src0.Serialize( stream );
        src1.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );

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

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        filename.Serialize( stream );
	}

private:
	SORT_MATERIAL_DEFINE_PROP_STR( filename );

	ImageTexture image_tex;
};
