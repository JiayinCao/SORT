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

class SORTNodeComposite : public MaterialNode
{
public:
    DEFINE_CREATOR( SORTNodeComposite , MaterialNode , "SORTNodeComposite" );
    
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_COLOR; }
    
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "R" , R );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "G" , G );
    SORT_MATERIAL_DEFINE_PROP_FLOAT( "B" , B );
};

class SORTNodeExtractRed : public MaterialNode
{
public:
    DEFINE_CREATOR( SORTNodeExtractRed , MaterialNode , "SORTNodeExtractRed" );
    
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_FLOAT; }
    
    void GetMaterialProperty( Bsdf* bsdf , float& result ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color" , color );
};

class SORTNodeExtractGreen : public MaterialNode
{
public:
    DEFINE_CREATOR( SORTNodeExtractGreen , MaterialNode , "SORTNodeExtractGreen" );
    
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_FLOAT; }
    
    void GetMaterialProperty( Bsdf* bsdf , float& result ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color" , color );
};

class SORTNodeExtractBlue : public MaterialNode
{
public:
    DEFINE_CREATOR( SORTNodeExtractBlue , MaterialNode , "SORTNodeExtractBlue" );
    
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_FLOAT; }
    
    void GetMaterialProperty( Bsdf* bsdf , float& result ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color" , color );
};

class SORTNodeExtractIntensity : public MaterialNode
{
public:
    DEFINE_CREATOR( SORTNodeExtractIntensity , MaterialNode , "SORTNodeIntensity" );
    
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_FLOAT; }
    
    void GetMaterialProperty( Bsdf* bsdf , float& result ) override;
    
private:
    SORT_MATERIAL_DEFINE_PROP_COLOR( "Color" , color );
};
