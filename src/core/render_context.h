/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#include <memory>
#include "core/define.h"
#include "core/memory.h"
#include "core/rand.h"

struct Qbvh_Node;
struct Obvh_Node;

//! @brief  Render context is the context for rendering for each fiber/thread
/**
 * With the introduction of fiber based job system, it is not possible to use TLS anymore
 * since fiber can be ran on different thread, thread local storage will not be the solution to 
 * lots of problems in SORT. As a matter of fact, there will be no easy way to use global memory
 * unless using fiber local storage. Instead of relying on a new feature, which I have no idea how
 * well it is supported on different platforms, I would prefer to introduce this concept named
 * render context, which captures all previously global accessible data through thread local storgae.
 * Each fiber/thread will have its own render context and once it is done, this context can be reused
 * by other job instance for better efficiency.
 */
struct RenderContext{
    //! Memory allocator for small piece of memory
    std::unique_ptr<MemoryAllocator>                m_memory_arena;

    std::unique_ptr<std::pair<Qbvh_Node*, float>[]> m_fast_qbvh_stack;
    std::unique_ptr<Qbvh_Node*[]>                   m_fast_qbvh_stack_simple;

    std::unique_ptr<std::pair<Obvh_Node*, float>[]> m_fast_obvh_stack;
    std::unique_ptr<Obvh_Node*[]>                   m_fast_obvh_stack_simple;

    std::unique_ptr<RandomNumberGenerator>          m_random_num_generator;

    //! @brief  Initialize the render context, only needs to be done once.
    void Init(){
        m_memory_arena = std::make_unique<MemoryAllocator>();
        m_random_num_generator = std::make_unique<RandomNumberGenerator>();

        m_fast_qbvh_stack = nullptr;
        m_fast_qbvh_stack_simple = nullptr;
        m_fast_obvh_stack = nullptr;
        m_fast_obvh_stack_simple = nullptr;
    }

    //! @brief  If the render context is initialized
    bool IsInitialized() const {
        return m_memory_arena != nullptr;
    }

    //! @brief  Reset the context so that it can be shared with future job instance
    RenderContext& Reset(){
        m_memory_arena->Reset();
        return *this;
    }
};