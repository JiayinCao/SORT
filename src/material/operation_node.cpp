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

#include "operation_node.h"

IMPLEMENT_CREATOR( SORTNodeOneMinus );
IMPLEMENT_CREATOR( AddNode );
IMPLEMENT_CREATOR( LerpNode );
IMPLEMENT_CREATOR( BlendNode );
IMPLEMENT_CREATOR( MutiplyNode );
IMPLEMENT_CREATOR( GammaToLinearNode );
IMPLEMENT_CREATOR( LinearToGammaNode );
IMPLEMENT_CREATOR( NormalDecoderNode );

void AddNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    result = c0 + c1;
}

void SORTNodeOneMinus::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c,src);
    result = Spectrum( 1.0f ) - c;
}

void LerpNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    SORT_MATERIAL_GET_PROP_FLOAT(f,factor);
    result = lerp( c0 , c1 , f );
}

void BlendNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    SORT_MATERIAL_GET_PROP_FLOAT(f0,factor0);
    SORT_MATERIAL_GET_PROP_FLOAT(f1,factor1);
    result = c0 * f0 + c1 * f1;
}

void MutiplyNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    result = c0 * c1;
}

void GammaToLinearNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c,src);
    result = Spectrum( GammaToLinear(c.GetR()) , GammaToLinear(c.GetG()) , GammaToLinear(c.GetB()) );
}

void LinearToGammaNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_COLOR(c,src);
    result = Spectrum( LinearToGamma(c.GetR()) , LinearToGamma(c.GetG()) , LinearToGamma(c.GetB()) );
}

void NormalDecoderNode::GetMaterialProperty( Bsdf* bsdf , Vector& result ){
    SORT_MATERIAL_GET_PROP_COLOR(n,src);
    result = Vector( n.GetR() * 2.0f - 1.0f , n.GetB() * 2.0f - 1.0f , n.GetG() * 2.0f - 1.0f );
}
