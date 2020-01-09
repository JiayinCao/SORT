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

#ifdef _MSC_VER
    #define SORT_FORCEINLINE __forceinline
#elif defined(__GNUC__)
    #define SORT_FORCEINLINE inline __attribute__((__always_inline__))
#else
    #define SORT_FORCEINLINE inline
#endif

#define SORT_STATIC_FORCEINLINE     static SORT_FORCEINLINE

// This feature replaces SSS with small mean free path with Lambert, it greatly reduces noises in SSS implementation.
// However, it does introduces more fireflies, which is not acceptable, for which reason it is disabled by default.
// The firefly is caused by multiple path with more bounces so that it leads to very low pdf because of each bounce.
// Blending a red diffuse and Cyan will also results in the same firefly problem. Further investigation needs to be
// done before enabling this feature.
// #define SSS_REPLACE_WITH_LAMBERT

// By default, transparent shadow is enabled. Meaning transparent material node will cast transparent shadow.
// This is not exactly physically based since the introduction of semi-transparent feature is not strictly physically
// based from the very beginning. Disable transparent shadow means that transparent geometry will also cast shadow
// even if it is fully transparent. I could have implemented extra optimization to avoid casting shadows for fully
// transparent geometry at the cost of introducing more complex design. However, in order to keep it simple and
// maintainable by myself, I would skip it.
// Performance wise, this feature does introduce quite some extra cost since all intersection tests need to also
// evaluate intersection results, like position, normal, uv, basically everything that is used in shader evaluation,
// when transparent material is present.
#define ENABLE_TRANSPARENT_SHADOW