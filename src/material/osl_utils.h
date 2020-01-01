/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// This file wraps most of OSL related interface

#pragma once

#include <OSL/oslexec.h>
#include "core/define.h"

SORT_FORCEINLINE bool isBlack( const OSL::Vec3& v ){
    return ( v.x == 0.0f ) && ( v.y == 0.0f ) && ( v.z == 0.0f );
}

SORT_FORCEINLINE float intensityOSLVec3( const OSL::Vec3& v ){
    static const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
    return YWeight[0] * v.x + YWeight[1] * v.y + YWeight[2] * v.z;
}