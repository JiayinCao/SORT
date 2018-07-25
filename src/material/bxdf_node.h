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
	// get node type
    MAT_NODE_TYPE getNodeType() override { return MAT_NODE_BXDF | MaterialNode::getNodeType(); }

	// check validation
    bool CheckValidation() override;
};

// Lambert node
class LambertNode : public BxdfNode
{
public:
	DEFINE_CREATOR( LambertNode , MaterialNode , "SORTNode_BXDF_Lambert" );

	// constructor
	LambertNode();

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
};

// Oren nayar node
class OrenNayarNode : public BxdfNode
{
public:
	DEFINE_CREATOR( OrenNayarNode , MaterialNode , "SORTNode_BXDF_OrenNayar" );

	// constructor
	OrenNayarNode();

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughness;
};

// Microfacet node
class MicrofacetReflectionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetReflectionNode , MaterialNode , "SORTNode_BXDF_MicrofacetReflection" );

	// constructor
	MicrofacetReflectionNode();
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughnessU;
    MaterialNodeProperty    roughnessV;
	MaterialNodeProperty	eta;
	MaterialNodeProperty	k;
	MaterialNodePropertyString	mf_dist;
};

// Microfacet node
class MicrofacetRefractionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetRefractionNode , MaterialNode , "SORTNode_BXDF_MicrofacetRefraction" );

	// constructor
	MicrofacetRefractionNode();
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
    MaterialNodeProperty	roughnessU;
    MaterialNodeProperty	roughnessV;
	MaterialNodeProperty	in_ior;
	MaterialNodeProperty	ext_ior;
	MaterialNodePropertyString	mf_dist;
};


// AshikhmanShirley node
class AshikhmanShirleyNode : public BxdfNode
{
public:
    DEFINE_CREATOR(AshikhmanShirleyNode, MaterialNode, "SORTNode_BXDF_AshikhmanShirley");

    // constructor
    AshikhmanShirleyNode();
    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;

private:
    MaterialNodeProperty	diffuse;
    MaterialNodeProperty	specular;
    MaterialNodeProperty	roughnessU;
    MaterialNodeProperty	roughnessV;
};

// Merl node
class MerlNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MerlNode , MaterialNode , "SORTNode_BXDF_MERL" );

    // constructor
    MerlNode();
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

    // post process
    void PostProcess() override;
    
private:
    MaterialNodePropertyString    merlfile;

    // the merl data
    Merl merl;
};

// Fourier bxdf node
class FourierBxdfNode : public BxdfNode
{
public:
    DEFINE_CREATOR( FourierBxdfNode , MaterialNode , "SORTNode_BXDF_Fourier" );
    
    // constructor
    FourierBxdfNode();

    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

    // post process
    void PostProcess() override;

private:
    MaterialNodePropertyString  fourierBxdfFile;

    // fourier bxdf node
    FourierBxdf fourierBxdf;
};
