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

// Layered BXDF node
class LayeredBxdfNode : public BxdfNode
{
public:
    DEFINE_CREATOR( LayeredBxdfNode , MaterialNode, "SORTNodeLayeredBxdf" );
    
    // constructor
    LayeredBxdfNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    // check validation
    bool CheckValidation() override;
    
private:
    MaterialNodeProperty    bxdfs[MAX_BXDF_COUNT];
    MaterialNodeProperty    weights[MAX_BXDF_COUNT];
};

// Lambert node
class LambertNode : public BxdfNode
{
public:
	DEFINE_CREATOR( LambertNode , MaterialNode , "SORTNodeLambert" );

	// constructor
	LambertNode();

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
};

// Merl node
class MerlNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MerlNode , MaterialNode , "SORTNodeMerl" );

	// constructor
	MerlNode();

	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

	// post process
    void PostProcess() override;

private:
	MaterialNodePropertyString	merlfile;

	// the merl data
	Merl merl;
};

// Fourier bxdf node
class FourierBxdfNode : public BxdfNode
{
public:
    DEFINE_CREATOR( FourierBxdfNode , MaterialNode , "SORTNodeFourierBxdf" );
    
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

// Oren nayar node
class OrenNayarNode : public BxdfNode
{
public:
	DEFINE_CREATOR( OrenNayarNode , MaterialNode , "SORTNodeOrenNayar" );

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
	DEFINE_CREATOR( MicrofacetReflectionNode , MaterialNode , "SORTNodeMicrofacetReflection" );

	// constructor
	MicrofacetReflectionNode();
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughness;
	MaterialNodeProperty	eta;
	MaterialNodeProperty	k;
	MaterialNodePropertyString	mf_dist;
	MaterialNodePropertyString	mf_vis;
};

// Microfacet node
class MicrofacetRefractionNode : public BxdfNode
{
public:
	DEFINE_CREATOR( MicrofacetRefractionNode , MaterialNode , "SORTNodeMicrofacetRefraction" );

	// constructor
	MicrofacetRefractionNode();
	// update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;

private:
	MaterialNodeProperty	baseColor;
	MaterialNodeProperty	roughness;
	MaterialNodeProperty	in_ior;
	MaterialNodeProperty	ext_ior;
	MaterialNodePropertyString	mf_dist;
	MaterialNodePropertyString	mf_vis;
};
