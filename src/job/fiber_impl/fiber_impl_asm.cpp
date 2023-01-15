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

#include "../fiber.h"

#if defined(SORT_ARM64_TARGET) && defined(SORT_IN_MAC)

static inline void RunFiber(Fiber* fiber){
    if(fiber->m_target_func)
        fiber->m_target_func();
}

Fiber::~Fiber() {
    if(m_stack_ptr){
        free_aligned(m_stack_ptr);
        m_stack_ptr = nullptr;
    }
}

void Fiber::Reset(const std::function<void()>& func){
    m_target_func = func;

    create_fiber(&m_context, m_stack_ptr, static_cast<uint32_t>(m_stack_size),
                 reinterpret_cast<void (*)(void*)>(RunFiber), this);
}

std::unique_ptr<Fiber> createFiberFromThread(){
    std::unique_ptr<Fiber> ptr = std::make_unique<Fiber>();
    ptr->m_context = {};
    return std::move(ptr);
}

std::unique_ptr<Fiber> createFiber(unsigned int stackSize, 
                                   const std::function<void()>& func) {
    if(stackSize == 0)
        return nullptr;

    std::unique_ptr<Fiber> ptr = std::make_unique<Fiber>();
    ptr->m_context = {};
    ptr->m_target_func = func;
    ptr->m_stack_ptr = malloc_aligned(stackSize, 16);
    ptr->m_stack_size = stackSize;
    create_fiber(&ptr->m_context, ptr->m_stack_ptr, static_cast<uint32_t>(stackSize),
                 reinterpret_cast<void (*)(void*)>(RunFiber), ptr.get());

    return std::move(ptr);
}

void switchFiber(Fiber* fromFiber, const Fiber* toFiber) {
    switch_fiber(&fromFiber->m_context, &toFiber->m_context);
}

#endif