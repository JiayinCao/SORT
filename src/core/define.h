/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
    #define SORT_IN_WINDOWS
#elif defined(__linux__)
    #define SORT_IN_LINUX

    // Make sure the compiler is C++14 compatible. Otherwise, make it clear that it is necessary to compile SORT in an error message.
    #if (__cplusplus < 201300L)
        #  error "SORT heavily uses features of C++14/11, please make sure you have a C++14 compatible compiler."
    #endif
#elif defined(__APPLE__)
    #define SORT_IN_MAC

    // Make sure the compiler is C++14 compatible. Otherwise, make it clear that it is necessary to compile SORT in an error message.
    #if (__cplusplus < 201300L)
        #  error "SORT heavily uses features of C++14/11, please make sure you have a C++14 compatible compiler."
    #endif
#endif

#ifdef SORT_IN_WINDOWS
    #define UNLIKELY(EXP)       (EXP)
    #define LIKELY(EXP)         (EXP)
#else
    #define UNLIKELY(EXP)       __builtin_expect((EXP),0)
    #define LIKELY(EXP)         __builtin_expect((EXP),1)
#endif

// This feature replaces SSS with small mean free path with Lambert, it greatly reduces noises in SSS implemenation.
// However, it does introduces more fireflies, which is not acceptable, for which reason it is disabled by default.
// The firefly is caused by multiple path with more bounces so that it leads to very low pdf because of each bounce.
// Blending a red diffuse and Cyan will also results in the same firefly problem. Further investigation needs to be
// done before enabling this feature.
// #define SSS_REPLACE_WITH_LAMBERT