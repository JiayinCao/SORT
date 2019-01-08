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

//! @brief  Operator node.
class OperatorNode : public MaterialNode{
public:
    //! @brief  Get the type of the material node.
    //!
    //! @return         Material node type.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_COLOR;
    }
};

//! @brief  Add two color.
class AddNode : public OperatorNode{
public:
	DEFINE_RTTI( AddNode , MaterialNode );

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
    void Serialize( IStreamBase& stream ) override {
        src0.Serialize( stream );
        src1.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );
};

//! @brief  Get the inverse ( 1 - src ) color.
class SORTNodeOneMinus : public OperatorNode{
public:
    DEFINE_RTTI( SORTNodeOneMinus , MaterialNode );
    
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
    void Serialize( IStreamBase& stream ) override {
        src.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src );
};

//! @brief  Lerp node.
class LerpNode : public OperatorNode{
public:
	DEFINE_RTTI( LerpNode , MaterialNode );

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
    void Serialize( IStreamBase& stream ) override {
        src0.Serialize( stream );
        src1.Serialize( stream );
        factor.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( factor );
};

//! @brief  Blend node.
class BlendNode : public OperatorNode{
public:
	DEFINE_RTTI( BlendNode , MaterialNode );

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
    void Serialize( IStreamBase& stream ) override {
        src0.Serialize( stream );
        src1.Serialize( stream );
        factor0.Serialize( stream );
        factor1.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( factor0 );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( factor1 );
};

//! @brief  Mutiply two spectrums.
class MutiplyNode : public OperatorNode{
public:
	DEFINE_RTTI( MutiplyNode , MaterialNode );
    
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
    void Serialize( IStreamBase& stream ) override {
        src0.Serialize( stream );
        src1.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src0 );
    SORT_MATERIAL_DEFINE_PROP_COLOR( src1 );
};

//! @brief  Gamma correction Node.
class GammaToLinearNode : public OperatorNode{
public:
    DEFINE_RTTI( GammaToLinearNode , MaterialNode );
    
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
    void Serialize( IStreamBase& stream ) override {
        src.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src );
};

//! @brief  Gamma correction Node.
class LinearToGammaNode : public OperatorNode{
public:
    DEFINE_RTTI( LinearToGammaNode , MaterialNode );
    
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
    void Serialize( IStreamBase& stream ) override {
        src.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src );
};

//! @brief  Normal decoding node.
class NormalDecoderNode : public OperatorNode{
public:
    DEFINE_RTTI( NormalDecoderNode , MaterialNode );
    
    //! @brief  Get vector material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Vector data structure to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Vector& result ) override;
    
    //! @brief  Get the type of the material node.
    //!
    //! @return         Material node type.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_VECTOR; }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        src.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( src );
};
