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
IMPLEMENT_RTTI( SORTNodeExtract );

IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( Result , SORTNodeComposite )
    SORT_MATERIAL_GET_PROP_FLOAT(r,R);
    SORT_MATERIAL_GET_PROP_FLOAT(g,G);
    SORT_MATERIAL_GET_PROP_FLOAT(b,B);
    result.SetColor(r, g, b);
IMPLEMENT_OUTPUT_FLOAT_SOCKET_END

IMPLEMENT_OUTPUT_FLOAT_SOCKET_BEGIN( Red , SORTNodeExtract )
    SORT_MATERIAL_GET_PROP_COLOR(c,color);
    result = c.GetR();
IMPLEMENT_OUTPUT_FLOAT_SOCKET_END

IMPLEMENT_OUTPUT_FLOAT_SOCKET_BEGIN( Green , SORTNodeExtract )
    SORT_MATERIAL_GET_PROP_COLOR(c,color);
    result = c.GetG();
IMPLEMENT_OUTPUT_FLOAT_SOCKET_END

IMPLEMENT_OUTPUT_FLOAT_SOCKET_BEGIN( Blue , SORTNodeExtract )
    SORT_MATERIAL_GET_PROP_COLOR(c,color);
    result = c.GetB();
IMPLEMENT_OUTPUT_FLOAT_SOCKET_END

IMPLEMENT_OUTPUT_FLOAT_SOCKET_BEGIN( Intensity , SORTNodeExtract )
    SORT_MATERIAL_GET_PROP_COLOR(c,color);
    result = c.GetIntensity();
IMPLEMENT_OUTPUT_FLOAT_SOCKET_END