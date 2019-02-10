/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include "texture/imagetexture.h"

//! @brief  Base class for texture node.
class TextureNode : public MaterialNode{
    //! @brief  Get the type of the material node.
    //!
    //! @return         Material node type.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_COLOR;
    }
};

//! @brief  Grid texture Node.
class GridTexNode : public TextureNode{
public:
	DEFINE_RTTI( GridTexNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , GridTexNode );

    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        src0.Serialize( stream , cache );
        src1.Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );
};

//! @brief  Grid texture Node.
class CheckerBoardTexNode : public TextureNode{
public:
	DEFINE_RTTI( CheckerBoardTexNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , CheckerBoardTexNode );

    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        src0.Serialize( stream , cache );
        src1.Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );
};

//! @brief  Image texture node.
class ImageTexNode : public TextureNode{
public:
	DEFINE_RTTI( ImageTexNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , ImageTexNode );

    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
	//! Load texture from tile.
    void PostProcess() override;

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        filename.Serialize( stream , cache );
	}

private:
	SORT_MATERIAL_DEFINE_PROP_STR( filename );

    /**< The image texture. */
	ImageTexture m_imageTexture;
};
