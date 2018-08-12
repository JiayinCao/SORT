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
    
    // constructor
    DisneyPrincipleNode();
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty        basecolor;
    MaterialNodeProperty        subsurface;
    MaterialNodeProperty        metallic;
    MaterialNodeProperty        specular;
    MaterialNodeProperty        specularTint;
    MaterialNodeProperty        roughness;
    MaterialNodeProperty        anisotropic;
    MaterialNodeProperty        sheen;
    MaterialNodeProperty        sheenTint;
    MaterialNodeProperty        clearcoat;
    MaterialNodeProperty        clearcoatGloss;
};

// Layered Material
class LayeredMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( LayeredMaterialNode , MaterialNode, "SORTNode_Material_Layered" );
    
    // constructor
    LayeredMaterialNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty    bxdfs[MAX_BXDF_COUNT];
    MaterialNodeProperty    weights[MAX_BXDF_COUNT];
};

// Priciple Material
class PrincipleMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( PrincipleMaterialNode , MaterialNode, "SORTNode_Material_Principle" );
    
    // constructor
    PrincipleMaterialNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty    baseColor;
    MaterialNodeProperty    roughnessU;
    MaterialNodeProperty    roughnessV;
    MaterialNodeProperty    metallic;
    MaterialNodeProperty    specular;
};

// Matte Material
class MatteMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MatteMaterialNode , MaterialNode, "SORTNode_Material_Matte" );
    
    // constructor
    MatteMaterialNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty    baseColor;
    MaterialNodeProperty    roughness;
};

// Plastic Material
class PlasticMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( PlasticMaterialNode , MaterialNode, "SORTNode_Material_Plastic" );
    
    // constructor
    PlasticMaterialNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty    diffuse;
    MaterialNodeProperty    specular;
    MaterialNodeProperty    roughness;
};

// Glass Material
class GlassMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( GlassMaterialNode , MaterialNode, "SORTNode_Material_Glass" );
    
    // constructor
    GlassMaterialNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty    reflectance;
    MaterialNodeProperty    transmittance;
    MaterialNodeProperty    roughnessU;
    MaterialNodeProperty    roughnessV;
};

// Mirror Material
class MirrorMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MirrorMaterialNode , MaterialNode, "SORTNode_Material_Mirror" );
    
    // constructor
    MirrorMaterialNode();
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    MaterialNodeProperty    basecolor;
};

// Measured Material
class MeasuredMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MeasuredMaterialNode , MaterialNode , "SORTNode_Material_Measured" );
    
    // constructor
    MeasuredMaterialNode();
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    // post process
    void PostProcess() override;
    
private:
    MaterialNodePropertyString  bxdfFilePath;
    MaterialNodePropertyString  bxdfType;
    
    FourierBxdfData fourierBxdfData;
    MerlData        merlData;
};
