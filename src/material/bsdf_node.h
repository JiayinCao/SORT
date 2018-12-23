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

#include "bxdf_node.h"
#include "bsdf/bsdf.h"
#include "bsdf/merl.h"
#include "bsdf/fourierbxdf.h"

// Disney Principle BRDF node
class DisneyPrincipleNode : public BxdfNode
{
public:
    DEFINE_CREATOR( DisneyPrincipleNode , MaterialNode , "SORTNode_Material_DisneyBRDF" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        subsurface.Serialize( stream );
        metallic.Serialize( stream );
        specular.Serialize( stream );
        specularTint.Serialize( stream );
        roughness.Serialize( stream );
        anisotropic.Serialize( stream );
        sheen.Serialize( stream );
        sheenTint.Serialize( stream );
        clearcoat.Serialize( stream );
        clearcoatGloss.Serialize( stream );
        basecolor.Serialize( stream );

        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
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

// Priciple Material
class PrincipleMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( PrincipleMaterialNode , MaterialNode, "SORTNode_Material_Principle" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        roughnessU.Serialize( stream );
        roughnessV.Serialize( stream );
        metallic.Serialize( stream );
        specular.Serialize( stream );
        baseColor.Serialize( stream );

        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessV );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( metallic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( specular );
};

// Matte Material
class MatteMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MatteMaterialNode , MaterialNode, "SORTNode_Material_Matte" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        baseColor.Serialize( stream );
        roughness.Serialize( stream );

        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
};

// Plastic Material
class PlasticMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( PlasticMaterialNode , MaterialNode, "SORTNode_Material_Plastic" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        diffuse.Serialize( stream );
        specular.Serialize( stream );
        roughness.Serialize( stream );

        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( diffuse );
    SORT_MATERIAL_DEFINE_PROP_COLOR( specular );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughness );
};

// Glass Material
class GlassMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( GlassMaterialNode , MaterialNode, "SORTNode_Material_Glass" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        reflectance.Serialize( stream );
        transmittance.Serialize( stream );
        roughnessU.Serialize( stream );
        roughnessV.Serialize( stream );

        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( reflectance );
    SORT_MATERIAL_DEFINE_PROP_COLOR( transmittance );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( roughnessV );
};

// Mirror Material
class MirrorMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MirrorMaterialNode , MaterialNode, "SORTNode_Material_Mirror" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        basecolor.Serialize( stream );
        BxdfNode::Serialize( stream );
    }

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( basecolor );
};

// Measured Material
class MeasuredMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MeasuredMaterialNode , MaterialNode , "SORTNode_Material_Measured" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    // post process
    void PostProcess() override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        bxdfType.Serialize( stream );
        bxdfFilePath.Serialize( stream );
        BxdfNode::Serialize( stream );
    }

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_STR( bxdfType );
    SORT_MATERIAL_DEFINE_PROP_STR( bxdfFilePath );
    
    FourierBxdfData fourierBxdfData;
    MerlData        merlData;
};

// Blend Material
class BlendMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR(BlendMaterialNode, MaterialNode, "SORTNode_Material_Blend");

    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        bxdf0.Serialize( stream );
        bxdf1.Serialize( stream );
        factor.Serialize( stream );
        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf0);
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf1);
    SORT_MATERIAL_DEFINE_PROP_FLOAT(factor);
};

// Double-Sided Material
class DoubleSidedMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR(DoubleSidedMaterialNode, MaterialNode, "SORTNode_Material_DoubleSided");

    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        bxdf0.Serialize( stream );
        bxdf1.Serialize( stream );
        BxdfNode::Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf0);
    SORT_MATERIAL_DEFINE_PROP_BXDF(bxdf1);
};
