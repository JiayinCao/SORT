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
#include "bsdf/bsdf.h"
#include "bsdf/merl.h"
#include "bsdf/fourierbxdf.h"

//! @brief  Base class for BXDF node.
class BxdfNode : public MaterialNode{
protected:
    //! @brief  Get the type of the material node.
    //!
    //! @return         Material node type.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_BXDF;
    }
    
    SORT_MATERIAL_DEFINE_PROP_VECTOR( normal );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        normal.Serialize( stream , cache );
	}
};

//! @brief  Lambert node.
class LambertNode : public BxdfNode{
public:
	DEFINE_RTTI( LambertNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , LambertNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        baseColor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
	SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
};

//! @brief Lambert transmitance node.
class LambertTransmissionNode : public BxdfNode{
public:
    DEFINE_RTTI( LambertTransmissionNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , LambertTransmissionNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        baseColor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
};

//! @brief  OrenNayar node.
class OrenNayarNode : public BxdfNode{
public:
	DEFINE_RTTI( OrenNayarNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , OrenNayarNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        roughness.Serialize( stream , cache );
        baseColor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
};

//! @brief  Microfacet reflection node.
class MicrofacetReflectionNode : public BxdfNode{
public:
	DEFINE_RTTI( MicrofacetReflectionNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , MicrofacetReflectionNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        mf_dist.Serialize( stream , cache );
        eta.Serialize( stream , cache );
        k.Serialize( stream , cache );
        roughnessU.Serialize( stream , cache );
        roughnessV.Serialize( stream , cache );
        baseColor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_STR( mf_dist );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessV );
    SORT_MATERIAL_DEFINE_PROP_COLOR( eta );
    SORT_MATERIAL_DEFINE_PROP_COLOR( k );
};

//! @brief  Microfacet refraction node.
class MicrofacetRefractionNode : public BxdfNode{
public:
	DEFINE_RTTI( MicrofacetRefractionNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , MicrofacetRefractionNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        mf_dist.Serialize( stream , cache );
        in_ior.Serialize( stream , cache );
        ext_ior.Serialize( stream , cache );
        roughnessU.Serialize( stream , cache );
        roughnessV.Serialize( stream , cache );
        baseColor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_STR( mf_dist );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessV );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( in_ior );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( ext_ior );
};

//! @brief AshikhmanShirley node.
class AshikhmanShirleyNode : public BxdfNode{
public:
    DEFINE_RTTI(AshikhmanShirleyNode, MaterialNode);
    DEFINE_OUTPUT_CHANNEL( Result , AshikhmanShirleyNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        specular.Serialize( stream , cache );
        roughnessU.Serialize( stream , cache );
        roughnessV.Serialize( stream , cache );
        diffuse.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR(diffuse);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(specular);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(roughnessU);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(roughnessV);
};

//! @brief  Phong node.
class PhongNode : public BxdfNode{
public:
    DEFINE_RTTI(PhongNode, MaterialNode);
    DEFINE_OUTPUT_CHANNEL( Result , PhongNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        power.Serialize( stream , cache );
        diffRatio.Serialize( stream , cache );
        specular.Serialize( stream , cache );
        diffuse.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR(diffuse);
    SORT_MATERIAL_DEFINE_PROP_COLOR(specular);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(power);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(diffRatio);
};

//! @brief  Merl node.
class MerlNode : public BxdfNode{
public:
    DEFINE_RTTI( MerlNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , MerlNode );
    
    //! @brief  Measured BRDF file is loaded in post-processing.
    void PostProcess() override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        merlfile.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_STR( merlfile );

    /**< The MERL BRDF data. */
    MerlData data;
};

//! @brief  Fourier bxdf node.
class FourierBxdfNode : public BxdfNode{
public:
    DEFINE_RTTI( FourierBxdfNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , FourierBxdfNode );

    //! @brief  Measured BRDF file is loaded in post-processing.
    void PostProcess() override;

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        fourierBxdfFile.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_STR( fourierBxdfFile );

    // fourier bxdf node
    FourierBxdfData fourierBxdfData;
};

//! @brief  Coat node.
class CoatNode : public BxdfNode{
public:
    DEFINE_RTTI(CoatNode, MaterialNode);
    DEFINE_OUTPUT_CHANNEL( Result , CoatNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        ior.Serialize( stream , cache );
        thickness.Serialize( stream , cache );
        sigma.Serialize( stream , cache );
        roughness.Serialize( stream , cache );
        bxdf.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }
    
private:
    SORT_MATERIAL_DEFINE_PROP_FLOAT( thickness );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
    SORT_MATERIAL_DEFINE_PROP_COLOR( sigma );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( ior );
    SORT_MATERIAL_DEFINE_PROP_BXDF( bxdf );
};
