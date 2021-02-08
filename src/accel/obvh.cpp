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

#include "obvh.h"

#define OBVH_IMPEMENTATION
#define Fbvh                    Obvh
#define Fbvh_Node               Obvh_Node
#define m_fast_bvh_stack        m_fast_obvh_stack
#define m_fast_bvh_stack_simple m_fast_obvh_stack_simple

#ifdef SIMD_8WAY_ENABLED
#define SIMD_8WAY_IMPLEMENTATION
#define SIMD_BVH_IMPLEMENTATION
#endif

#include "fast_bvh.hpp"

#ifdef SIMD_8WAY_ENABLED
#undef SIMD_BVH_IMPLEMENTATION
#undef SIMD_8WAY_IMPLEMENTATION
#endif

#undef  Fbvh
#undef  Fbvh_Node
#undef  m_fast_bvh_stack
#undef  m_fast_bvh_stack_simple