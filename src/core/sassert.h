/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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

#include "core/define.h"
#include "core/log.h"
#include <assert.h>

// uncomment this line to add assert in release build
// #define FORCE_ASSERT

#if defined(SORT_DEBUG) || defined(FORCE_ASSERT)

#define sAssert(expr,type) \
    if( UNLIKELY(false == (bool)(expr)) ) {\
        slog( CRITICAL , type , "Crashed!" );\
        abort();\
    }

#define sAssertMsg(expr, type, ... )\
    if( UNLIKELY(false == (bool)(expr)) ){\
        slog( CRITICAL , type , __VA_ARGS__ );\
        abort();\
    }
    
#else

#define sAssert(expr,type)              {}
#define sAssertMsg(expr, type, ... )    {}

#endif