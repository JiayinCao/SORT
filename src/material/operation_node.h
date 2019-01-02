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

class OperatorNode : public MaterialNode
{
public:
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_COLOR; }
};

// Adding node
class AddNode : public OperatorNode
{
public:
	DEFINE_CREATOR( AddNode , MaterialNode , "SORTNodeAdd" );

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

// Inverse node
class SORTNodeOneMinus : public OperatorNode
{
public:
    DEFINE_CREATOR( SORTNodeOneMinus , MaterialNode , "SORTNodeOneMinus" );
    
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

// Lerp node
class LerpNode : public OperatorNode
{
public:
	DEFINE_CREATOR( LerpNode , MaterialNode , "SORTNodeLerp" );

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

// Blend node
class BlendNode : public OperatorNode
{
public:
	DEFINE_CREATOR( BlendNode , MaterialNode , "SORTNodeBlend" );

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

// Mutiply node
class MutiplyNode : public OperatorNode
{
public:
	DEFINE_CREATOR( MutiplyNode , MaterialNode , "SORTNodeMultiply" );
    
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

// Gamma correction Node
class GammaToLinearNode : public OperatorNode
{
public:
    DEFINE_CREATOR( GammaToLinearNode , MaterialNode , "SORTNodeGammaToLinear" );
    
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

// Gamma correction Node
class LinearToGammaNode : public OperatorNode
{
public:
    DEFINE_CREATOR( LinearToGammaNode , MaterialNode , "SORTNodeLinearToGamma" );
    
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

// Gamma correction Node
class NormalDecoderNode : public OperatorNode
{
public:
    DEFINE_CREATOR( NormalDecoderNode , MaterialNode , "SORTNodeDecodeNormal" );
    
    void GetMaterialProperty( Bsdf* bsdf , Vector& result ) override;
    
    // get node return type
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
