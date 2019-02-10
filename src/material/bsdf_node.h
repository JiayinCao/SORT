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

#include "bxdf_node.h"
#include "bsdf/bsdf.h"
#include "bsdf/merl.h"
#include "bsdf/fourierbxdf.h"

//! @brief  Disney Principle BRDF node
class DisneyPrincipleNode : public BxdfNode{
public:
    DEFINE_RTTI( DisneyPrincipleNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , DisneyPrincipleNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        subsurface.Serialize( stream , cache );
        metallic.Serialize( stream , cache );
        specular.Serialize( stream , cache );
        specularTint.Serialize( stream , cache );
        roughness.Serialize( stream , cache );
        anisotropic.Serialize( stream , cache );
        sheen.Serialize( stream , cache );
        sheenTint.Serialize( stream , cache );
        clearcoat.Serialize( stream , cache );
        clearcoatGloss.Serialize( stream , cache );
        basecolor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( basecolor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( subsurface );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( metallic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( specular );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( specularTint );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( anisotropic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( sheen );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( sheenTint );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( clearcoat );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( clearcoatGloss );
};

//! Principle Material is very similiar with UE4 shading model.
class PrincipleMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI( PrincipleMaterialNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , PrincipleMaterialNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        roughnessU.Serialize( stream , cache );
        roughnessV.Serialize( stream , cache );
        metallic.Serialize( stream , cache );
        specular.Serialize( stream , cache );
        baseColor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessV );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( metallic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( specular );
};

//! @brief  Matte Material.
/**
 * Depending on the roughness, this material could be pure lambert BRDF or OrenNayar BRDF.
 */
class MatteMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI( MatteMaterialNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , MatteMaterialNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        baseColor.Serialize( stream , cache );
        roughness.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
};

//! @brief  Plastic material is a combination of lambert BRDF and microfacet BRDF.
class PlasticMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI( PlasticMaterialNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , PlasticMaterialNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        diffuse.Serialize( stream , cache );
        specular.Serialize( stream , cache );
        roughness.Serialize( stream , cache );

        BxdfNode::Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( diffuse );
    SORT_MATERIAL_DEFINE_PROP_COLOR( specular );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
};

//! @brief  Glass material node uses microfacet refraction model.
class GlassMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI( GlassMaterialNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , GlassMaterialNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        reflectance.Serialize( stream , cache );
        transmittance.Serialize( stream , cache );
        roughnessU.Serialize( stream , cache );
        roughnessV.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( reflectance );
    SORT_MATERIAL_DEFINE_PROP_COLOR( transmittance );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessV );
};

//! @brief  Mirror material is microfacet reflection model with 0 as roughness.
class MirrorMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI( MirrorMaterialNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , MirrorMaterialNode );
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        basecolor.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( basecolor );
};

//! @brief  Measured Material could be fourier or MERL brdf.
class MeasuredMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI( MeasuredMaterialNode , MaterialNode );
    DEFINE_OUTPUT_CHANNEL( Result , MeasuredMaterialNode );

    //! @brief  Measured BRDF file is loaded in post-processing.
    void PostProcess() override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        bxdfType.Serialize( stream , cache );
        bxdfFilePath.Serialize( stream , cache );
        BxdfNode::Serialize( stream , cache );
    }

private:
    SORT_MATERIAL_DEFINE_PROP_STR( bxdfType );
    SORT_MATERIAL_DEFINE_PROP_STR( bxdfFilePath );
    
    FourierBxdfData m_fourierBxdfData;        /**< Measured BRDF data in the form of fourier. */
    MerlData        m_merlData;               /**< Measured BRDF data in the form of MERL. */
};

//! @brief  Blend material node is used to implement layered material.
class BlendMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI(BlendMaterialNode, MaterialNode);
    DEFINE_OUTPUT_CHANNEL( Result , BlendMaterialNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        bxdf0.Serialize( stream , cache );
        bxdf1.Serialize( stream , cache );
        factor.Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf0);
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf1);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(factor);
};

//! @brief  Double-sided material node shows another BRDF on the other side of the primitive.
/**
 * It is up to the user to make sure valid materials are setup. There is no enough reason to check invalid material
 * setup using this node. It is totally possible to setup a BTDF on the other side to break the renderer. Users of
 * SORT should generally not do this.
 */
class DoubleSidedMaterialNode : public BxdfNode{
public:
    DEFINE_RTTI(DoubleSidedMaterialNode, MaterialNode);
    DEFINE_OUTPUT_CHANNEL( Result , DoubleSidedMaterialNode );

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override {
        bxdf0.Serialize( stream , cache );
        bxdf1.Serialize( stream , cache );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf0);
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf1);
};
