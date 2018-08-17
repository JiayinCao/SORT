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
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "BaseColor" ,      basecolor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "SubSurface" ,     subsurface );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Metallic" ,       metallic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Specular" ,       specular );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "SpecularTint" ,   specularTint );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Roughness" ,      roughness );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Anisotropic" ,    anisotropic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Sheen" ,          sheen );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "SheenTint" ,      sheenTint );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Clearcoat" ,      clearcoat );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "ClearcoatGloss" , clearcoatGloss );
};

// Priciple Material
class PrincipleMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( PrincipleMaterialNode , MaterialNode, "SORTNode_Material_Principle" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "BaseColor" , baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessU" , roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessV" , roughnessV );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Metallic" , metallic );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Specular" , specular );
};

// Matte Material
class MatteMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MatteMaterialNode , MaterialNode, "SORTNode_Material_Matte" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "BaseColor" , baseColor );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Roughness" , roughness );
};

// Plastic Material
class PlasticMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( PlasticMaterialNode , MaterialNode, "SORTNode_Material_Plastic" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Diffuse" , diffuse );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Specular" , specular );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "Roughness" , roughness );
};

// Glass Material
class GlassMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( GlassMaterialNode , MaterialNode, "SORTNode_Material_Glass" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Reflectance" , reflectance );
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Transmittance" , transmittance );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessU" , roughnessU );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "RoughnessV" , roughnessV );
};

// Mirror Material
class MirrorMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR( MirrorMaterialNode , MaterialNode, "SORTNode_Material_Mirror" );
    
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "BaseColor" , basecolor );
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
    
private:
    SORT_MATERIAL_DEFINE_PROP_STR( "Type" , bxdfType );
    SORT_MATERIAL_DEFINE_PROP_STR( "Filename" , bxdfFilePath );
    
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

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF("Bxdf0", bxdf0);
    SORT_MATERIAL_DEFINE_PROP_BXDF("Bxdf1", bxdf1);
    SORT_MATERIAL_DEFINE_PROP_FLOAT("Factor", factor);
};

// Double-Sided Material
class DoubleSidedMaterialNode : public BxdfNode
{
public:
    DEFINE_CREATOR(DoubleSidedMaterialNode, MaterialNode, "SORTNode_Material_DoubleSided");

    // update bsdf
    void UpdateBSDF(Bsdf* bsdf, Spectrum weight = 1.0f) override;

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF("Bxdf0", bxdf0);
    SORT_MATERIAL_DEFINE_PROP_BXDF("Bxdf1", bxdf1);
};
