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

#include "fiber.h"

bool create_fiber(const FiberInitDesc& desc, Fiber& fiber) {
#ifdef SORT_IN_WINDOWS
    if (fiber.m_fiber)
        return false;
    
    // Create the fiber
    fiber.m_fiber = ::CreateFiber(desc.m_stack_size, desc.m_fiber_func, desc.m_fiber_arg);

    return fiber.m_fiber != nullptr;
#else
    return true;
#endif
}

void switch_to_fiber(Fiber& fiber) {
#ifdef SORT_IN_WINDOWS
    SwitchToFiber(fiber.m_fiber);
#else

#endif
}

void fiber_from_thread(Fiber& fiber) {
#ifdef SORT_IN_WINDOWS
    fiber.m_fiber = ConvertThreadToFiber(0);
#else

#endif
}

void thread_from_fiber() {
#ifdef SORT_IN_WINDOWS
    ConvertFiberToThread();
#else

#endif
}

bool delete_fiber(Fiber& fiber) {
    if (!fiber.m_fiber)
        return false;

#ifdef SORT_IN_WINDOWS
    DeleteFiber(fiber.m_fiber);
    fiber.m_fiber = nullptr;
    return true;
#else
    return true;
#endif
}