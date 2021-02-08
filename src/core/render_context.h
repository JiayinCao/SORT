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

#include <tsl_version.h>
#include <tsl_system.h>
#include <memory>
#include "core/define.h"
#include "core/memory.h"

//! @brief  Render context is the context for rendering for each fiber/thread
/**
 * With the introduction of fiber based job system, it is not possible to use thread_local anymore
 * since fiber can be ran on different thread, thread_local storage will not be the solution to 
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
    //! Shading context for shader execution
    std::shared_ptr<Tsl_Namespace::ShadingContext>  m_shading_context;

    //! @brief  Initialize the render context, only needs to be done once.
    void Init(){
        m_memory_arena = std::make_unique<MemoryAllocator>();
        m_shading_context = Tsl_Namespace::ShadingSystem::get_instance().make_shading_context();
    }

    //! @brief  If the render context is initialized
    bool IsInitialized() const {
        return m_memory_arena && m_shading_context;
    }

    //! @brief  Reset the context so that it can be shared with future job instance
    void Reset(){
        m_memory_arena->Reset();
    }
};