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
#include "bsdf/bsdf.h"
#include "bsdf/merl.h"
#include "bsdf/fourierbxdf.h"

// Bxdf node
class BxdfNode : public MaterialNode
{
protected:
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_BXDF; }
    
    SORT_MATERIAL_DEFINE_PROP_VECTOR( "Normal" , normal );
};

// Lambert node
class LambertNode : public BxdfNode
{
public:
	DEFINE_CREATOR( LambertNode , MaterialNode , "SORTNode_BXDF_Lambert" );

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	SORT_MATERIAL_DEFINE_PROP_COLOR( "Diffuse" , baseColor );
};

// Lambert node
class LambertTransmissionNode : public BxdfNode
{
public:
    DEFINE_CREATOR( LambertTransmissionNode , MaterialNode , "SORTNode_BXDF_LambertTransmission" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Diffuse" , baseColor );
};

// Oren nayar node
class OrenNayarNode : public BxdfNode
{
public:
	DEFINE_CREATOR( OrenNayarNode , MaterialNode , "SORTNode_BXDF_OrenNayar" );

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Diffuse" , baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Roughness" , roughness );
};

// Microfacet node
class MicrofacetReflectionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetReflectionNode , MaterialNode , "SORTNode_BXDF_MicrofacetReflection" );

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "BaseColor" , baseColor );
    SORT_MATERIAL_DEFINE_PROP_STR( "MicroFacetDistribution" , mf_dist );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessU" , roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessV" , roughnessV );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Interior_IOR" , eta );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Absorption_Coefficient" , k );
};

// Microfacet node
class MicrofacetRefractionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetRefractionNode , MaterialNode , "SORTNode_BXDF_MicrofacetRefraction" );

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "BaseColor" , baseColor );
    SORT_MATERIAL_DEFINE_PROP_STR( "MicroFacetDistribution" , mf_dist );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessU" , roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessV" , roughnessV );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Interior_IOR" , in_ior );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Exterior_IOR" , ext_ior );
};

// AshikhmanShirley node
class AshikhmanShirleyNode : public BxdfNode
{
public:
    DEFINE_CREATOR(AshikhmanShirleyNode, MaterialNode, "SORTNode_BXDF_AshikhmanShirley");

    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR("Diffuse", diffuse);
    SORT_MATERIAL_DEFINE_PROP_FLOAT("Specular", specular);
    SORT_MATERIAL_DEFINE_PROP_FLOAT("RoughnessU", roughnessU);
    SORT_MATERIAL_DEFINE_PROP_FLOAT("RoughnessV", roughnessV);
};

// Phong node
class PhongNode : public BxdfNode
{
public:
    DEFINE_CREATOR(PhongNode, MaterialNode, "SORTNode_BXDF_Phong");

    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;

private:
    SORT_MATERIAL_DEFINE_PROP_COLOR("Diffuse", diffuse);
    SORT_MATERIAL_DEFINE_PROP_COLOR("Specular", specular);
    SORT_MATERIAL_DEFINE_PROP_FLOAT("SpecularPower", power);
    SORT_MATERIAL_DEFINE_PROP_FLOAT("DiffuseRatio", diffRatio);
};

// Merl node
class MerlNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MerlNode , MaterialNode , "SORTNode_BXDF_MERL" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

    // post process
    void PostProcess() override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_STR( "Filename" , merlfile );

    // the merl data
    MerlData data;
};

// Fourier bxdf node
class FourierBxdfNode : public BxdfNode
{
public:
    DEFINE_CREATOR( FourierBxdfNode , MaterialNode , "SORTNode_BXDF_Fourier" );

    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

    // post process
    void PostProcess() override;

private:
    SORT_MATERIAL_DEFINE_PROP_STR( "Filename" , fourierBxdfFile );

    // fourier bxdf node
    FourierBxdfData fourierBxdfData;
};

// Coat node
class CoatNode : public BxdfNode
{
public:
    DEFINE_CREATOR(CoatNode, MaterialNode, "SORTNode_BXDF_Coat");
    
    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Thickness" , thickness );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Roughness" , roughness );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Sigma" , sigma );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "IOR" , ior );
    SORT_MATERIAL_DEFINE_PROP_BXDF( "Surface" , bxdf );
};
