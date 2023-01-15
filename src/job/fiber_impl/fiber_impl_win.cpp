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

#include "core/define.h"

#if defined(SORT_IN_WINDOWS)

#include <Windows.h>
#include "../fiber.h"

void WINAPI RunFiber(PVOID arg) {
    Fiber* fiber = reinterpret_cast<Fiber*>(arg);
    if (fiber->m_target_func)
        fiber->m_target_func();
}

Fiber::~Fiber() {
    if (m_context.m_fiber != nullptr) {
        if (m_context.m_isFiberFromThread) {
            ConvertFiberToThread();
        }
        else {
            DeleteFiber(m_context.m_fiber);
        }

        m_context.m_fiber = nullptr;
        m_context.m_isFiberFromThread = false;
    }
}

std::unique_ptr<Fiber> createFiberFromThread() {
    auto ptr = std::make_unique<Fiber>();
    ptr->m_context.m_fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
    ptr->m_context.m_isFiberFromThread = true;
    return std::move(ptr);
}

std::unique_ptr<Fiber> createFiber(unsigned int stackSize,
                                   const std::function<void()>& func) {

    auto ptr = std::make_unique<Fiber>();

    // stackSize is rounded up to the system's allocation granularity (typically 64 KB).
    ptr->m_context.m_fiber = CreateFiberEx(stackSize - 1, stackSize, FIBER_FLAG_FLOAT_SWITCH,
                                           RunFiber, ptr.get());
    ptr->m_target_func = func;

    return std::move(ptr);
}

void switchFiber(Fiber* fromFiber, const Fiber* toFiber) {
    SwitchToFiber(toFiber->m_context.m_fiber);
}

#endif