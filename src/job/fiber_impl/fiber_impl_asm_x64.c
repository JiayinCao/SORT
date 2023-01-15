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

#include <stdint.h>
#include "core/define.h"

#if defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)

#include "fiber_impl_asm_x64.h"

// fiber entry function
static inline void fiber_entry(void (*target)(void*), void* arg) {
  target(arg);
}

// setup a new fiber
void create_fiber(struct FiberContext* context,
                  void* stack,
                  uint32_t stackSize,
                  void (*target)(void*),
                  void* arg) {
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stackSize);
    context->rip = (uintptr_t)&fiber_entry;
    context->rdi = (uintptr_t)target;
    context->rsi = (uintptr_t)arg;
    context->rsp = (uintptr_t)&stack_top[-3];
    stack_top[-2] = 0;  // No return target.
}

#endif