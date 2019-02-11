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

#include "operation_node.h"

IMPLEMENT_RTTI( SORTNodeOneMinus );
IMPLEMENT_RTTI( AddNode );
IMPLEMENT_RTTI( LerpNode );
IMPLEMENT_RTTI( BlendNode );
IMPLEMENT_RTTI( MutiplyNode );
IMPLEMENT_RTTI( GammaToLinearNode );
IMPLEMENT_RTTI( LinearToGammaNode );
IMPLEMENT_RTTI( NormalDecoderNode );

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , SORTNodeOneMinus )
    SORT_MATERIAL_GET_PROP_COLOR(c,src);
    result = Spectrum( 1.0f ) - c;
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , AddNode )
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    result = c0 + c1;
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , LerpNode )
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    SORT_MATERIAL_GET_PROP_FLOAT(f,factor);
    result = lerp( c0 , c1 , f );
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , BlendNode )
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    SORT_MATERIAL_GET_PROP_FLOAT(f0,factor0);
    SORT_MATERIAL_GET_PROP_FLOAT(f1,factor1);
    result = c0 * f0 + c1 * f1;
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , MutiplyNode )
    SORT_MATERIAL_GET_PROP_COLOR(c0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(c1,src1);
    result = c0 * c1;
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , GammaToLinearNode )
    SORT_MATERIAL_GET_PROP_COLOR(c,src);
    result = Spectrum( GammaToLinear(c.GetR()) , GammaToLinear(c.GetG()) , GammaToLinear(c.GetB()) );
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , LinearToGammaNode )
    SORT_MATERIAL_GET_PROP_COLOR(c,src);
    result = Spectrum( LinearToGamma(c.GetR()) , LinearToGamma(c.GetG()) , LinearToGamma(c.GetB()) );
IMPLEMENT_OUTPUT_COLOR_SOCKET_END

IMPLEMENT_OUTPUT_VEC_SOCKET_BEGIN( Result , NormalDecoderNode )
    SORT_MATERIAL_GET_PROP_COLOR(n,src);
    result = Vector( n.GetR() * 2.0f - 1.0f , n.GetB() * 2.0f - 1.0f , n.GetG() * 2.0f - 1.0f );
IMPLEMENT_OUTPUT_VEC_SOCKET_END