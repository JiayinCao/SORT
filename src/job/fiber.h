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

#include <functional>
#include "core/define.h"
#include "core/memory.h"

#if defined(SORT_IN_WINDOWS)
    #include "fiber_impl/fiber_impl_win.h"

    #define SORT_FIBER_ASM_IMPLEMENTATION   0
#elif defined(SORT_IN_LINUX)
    #include "fiber_impl/fiber_impl_linux.h"

    #define SORT_FIBER_ASM_IMPLEMENTATION   1
#elif defined(SORT_IN_MAC)
    #include "fiber_impl/fiber_impl_mac.h"

    #define SORT_FIBER_ASM_IMPLEMENTATION   1
#endif


//! @brief  Platform dependent fiber abstraction
/**
 * SORT's job system heavily uses fibers under the hood to support custom thread
 * scheduling and cheap thread switch. This all sits on the fiber implementation.
 */
struct Fiber {
    //! @brief  Destructor
    ~Fiber();

    // fiber context, this is platform dependent
    FiberContext            m_context;

    // real fiber function to be executed
    std::function<void()>   m_target_func;

#if SORT_FIBER_ASM_IMPLEMENTATION
    // pointer to stack
    void*                   m_stack_ptr = nullptr;

    // stack size
    unsigned int            m_stack_size = 0;
#endif
};

//! @brief  Convert the current thread to fiber.
//!
//! Note, on Ubuntu and Mac, this doesn't really convert the current thread to a
//! new fiber, it really just creates a brand new fiber that has nothing to do
//! with the current thread. However, as long as the thread first switch to a fiber
//! from this created fiber, this would allow the fiber to capture the thread
//! context and should be good enough for SORT's use case.
//!
//! @return Fiber created based on the current thread.
std::unique_ptr<Fiber> createFiberFromThread();

//! @brief  Create a new fiber.
//!
//! @param stackSize    The size of the stack for the newly created fiber.
//! @param func         Entry function of the fiber.
//! @return             Newly created fiber.
std::unique_ptr<Fiber> createFiber(unsigned int stackSize, const std::function<void()>& func = nullptr);

//! @brief Switch from one fiber to another different fiber
//!
//! Warning, the upper level logic should have checked for same fiber switch.
//! The low level implementation defined here won't do such a check. In the case of
//! switching to a same fiber, it should not crash the system. But there could be some
//! loss of performance.
//!
//! @param fromFiber    This fiber must the be executing fiber. It would potentially crash the 
//!                     renderer otherwise. And data in this fiber will be recaptured right
//!                     before the switch.
//! @param toFiber      This is the target fiber to take over the hardware core.
void switchFiber(Fiber* fromFiber, const Fiber* toFiber);