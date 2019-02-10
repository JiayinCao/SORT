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

#include "convertor_node.h"

IMPLEMENT_RTTI( SORTNodeComposite );
IMPLEMENT_RTTI( SORTNodeExtractRed );
IMPLEMENT_RTTI( SORTNodeExtractGreen );
IMPLEMENT_RTTI( SORTNodeExtractBlue );
IMPLEMENT_RTTI( SORTNodeExtractIntensity );
IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , SORTNodeComposite )
IMPLEMENT_OUTPUT_CHANNEL_END
IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , SORTNodeExtractRed )
IMPLEMENT_OUTPUT_CHANNEL_END
IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , SORTNodeExtractGreen )
IMPLEMENT_OUTPUT_CHANNEL_END
IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , SORTNodeExtractBlue )
IMPLEMENT_OUTPUT_CHANNEL_END
IMPLEMENT_OUTPUT_CHANNEL_BEGIN( Result , SORTNodeExtractIntensity )
IMPLEMENT_OUTPUT_CHANNEL_END

void SORTNodeComposite::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    SORT_MATERIAL_GET_PROP_FLOAT_TMP(r,R);
    SORT_MATERIAL_GET_PROP_FLOAT_TMP(g,G);
    SORT_MATERIAL_GET_PROP_FLOAT_TMP(b,B);
    result.SetColor(r, g, b);
}

void SORTNodeExtractRed::GetMaterialProperty( Bsdf* bsdf , float& result ){
    SORT_MATERIAL_GET_PROP_COLOR_TMP(c,color);
    result = c.GetR();
}

void SORTNodeExtractGreen::GetMaterialProperty( Bsdf* bsdf , float& result ){
    SORT_MATERIAL_GET_PROP_COLOR_TMP(c,color);
    result = c.GetG();
}

void SORTNodeExtractBlue::GetMaterialProperty( Bsdf* bsdf , float& result ){
    SORT_MATERIAL_GET_PROP_COLOR_TMP(c,color);
    result = c.GetB();
}

void SORTNodeExtractIntensity::GetMaterialProperty( Bsdf* bsdf , float& result ){
    SORT_MATERIAL_GET_PROP_COLOR_TMP(c,color);
    result = c.GetIntensity();
}
