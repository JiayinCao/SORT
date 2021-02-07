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

#include "core/define.h"

#ifdef SORT_IN_WINDOWS
#include <Windows.h>

using Native_Fiber_Handle = void*;
using Native_Fiber_FuncPtr = LPFIBER_START_ROUTINE;
using Native_Fiber_Arg = void*;
#define INVALID_FIBER   nullptr
#else
#include <sys/ucontext.h>

using Native_Fiber_Handle = ucontext_t;
using Native_Fiber_Arg = int;
using Native_Fiber_FuncPtr = void (*)(Native_Fiber_Arg);
#define INVALID_FIBER   ucontext_t()
#endif

struct FiberInitDesc {
    unsigned int            m_stack_size = 0;
    Native_Fiber_FuncPtr    m_fiber_func;
    void*                   m_fiber_arg;
};

struct Fiber {
    Native_Fiber_Handle m_fiber = INVALID_FIBER;
};

//! Create a fiber
bool create_fiber(const FiberInitDesc& desc, Fiber& fiber);

//! Switch from one fiber to another fiber
//! This has to be called in a fiber context on Windows.
void switch_to_fiber(Fiber& fiber);

//! Convert current thread to fiber
void fiber_from_thread(Fiber& fiber);

//! Convert the current fiber to thread
void thread_from_fiber();

//! Delete the fiber
bool delete_fiber(Fiber& fiber);