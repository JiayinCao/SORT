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
    #define _WINSOCKAPI_
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

// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
    #define SORT_X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
    #define SORT_X64_TARGET
#elif defined(__aarch64__)
    #define SORT_ARM64_TARGET
#elif
    #error "Undefined target."
#endif

#define SORT_STATIC_FORCEINLINE     static SORT_FORCEINLINE

#define IS_PTR_INVALID(p)           (nullptr == p)
#define IS_PTR_VALID(p)             (nullptr != p)

// This feature replaces SSS with small mean free path with Lambert, it greatly reduces noises in SSS implementation.
// However, it does introduces more fireflies, which is not acceptable, for which reason it is disabled by default.
// The firefly is caused by multiple path with more bounces so that it leads to very low pdf because of each bounce.
// Blending a red diffuse and Cyan will also results in the same firefly problem. Further investigation needs to be
// done before enabling this feature.
// #define SSS_REPLACE_WITH_LAMBERT

// By default, transparent shadow is enabled. Meaning transparent material node will cast transparent shadow.
// This is not exactly physically based since the introduction of semi-transparent feature is not strictly physically
// based from the very beginning. Disabling transparent shadow means that transparent geometry will also cast shadow
// even if it is fully transparent. I could have implemented extra optimization to avoid casting shadows for fully
// transparent geometry at the cost of introducing more complex design. However, in order to keep it simple and
// maintainable by myself, I would skip it.
// Performance wise, this feature does introduce quite some extra cost since all intersection tests need to also
// evaluate intersection results, like position, normal, uv, basically everything that is used in shader evaluation,
// when transparent material is present.
#define ENABLE_TRANSPARENT_SHADOW

// Multi-thread shader compilation.
// This is not a functional feature since there could be some data race problem in the TSL library. By default, this
// is disabled.
// Also, before I have a refactored job system with more flexible design so that I can spawn jobs inside a job, multi-
// thread shader compilation is not a done feature. The current solution is just the first iteration that only
// bulids material shader group in seperate thread since I know for a fact there is no dependencies between material
// shader group. However, in an ideal world, even shader unit should be compiled in a multi-thread environment to fully
// utilize the power of TSL's multi-thread compilation, this does require a more robust job system, which SORT currently
// doesn't have.
// This will be disabled since the efficiency of the current task system will be the main performance bottleneck,
// leading to worse performance with multi-thread shader compilation enabled.
// #define ENABLE_MULTI_THREAD_SHADER_COMPILATION

// This is a temporary quick solution to enable multi-thread texture loading. It is by no means a very good idea to 
// parallel a bunch of IO bound threads. However, my newly planned job system is far from being ready yet, I'll live 
// with it for now. This async loading eventually will be less useful since I'm planning to implement a texture cache
// system in the future to do lazy texture loading in the future.
#define ENABLE_ASYNC_TEXTURE_LOADING