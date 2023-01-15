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

#if defined(__aarch64__)

#include "fiber_impl_asm_aarch64_shared.h"

.text
.global ASM_ENTRY_WRAPPER(switch_fiber)
.align 4
ASM_ENTRY_WRAPPER(switch_fiber):
    str x16, [x0, #FIBER_REG_X16]
    str x17, [x0, #FIBER_REG_X17]
    str x18, [x0, #FIBER_REG_X18]

    str x19, [x0, #FIBER_REG_X19]
    str x20, [x0, #FIBER_REG_X20]
    str x21, [x0, #FIBER_REG_X21]
    str x22, [x0, #FIBER_REG_X22]
    str x23, [x0, #FIBER_REG_X23]
    str x24, [x0, #FIBER_REG_X24]
    str x25, [x0, #FIBER_REG_X25]
    str x26, [x0, #FIBER_REG_X26]
    str x27, [x0, #FIBER_REG_X27]
    str x28, [x0, #FIBER_REG_X28]
    str x29, [x0, #FIBER_REG_X29]

    str d8,  [x0, #FIBER_REG_V8]
    str d9,  [x0, #FIBER_REG_V9]
    str d10, [x0, #FIBER_REG_V10]
    str d11, [x0, #FIBER_REG_V11]
    str d12, [x0, #FIBER_REG_V12]
    str d13, [x0, #FIBER_REG_V13]
    str d14, [x0, #FIBER_REG_V14]
    str d15, [x0, #FIBER_REG_V15]

    // Store sp and lr
    mov x2, sp
    str x2,  [x0, #FIBER_REG_SP]
    str x30, [x0, #FIBER_REG_X30]

    // Load context 'to'
    mov x7, x1

    ldr x16, [x7, #FIBER_REG_X16]
    ldr x17, [x7, #FIBER_REG_X17]
    ldr x18, [x7, #FIBER_REG_X18]

    ldr x19, [x7, #FIBER_REG_X19]
    ldr x20, [x7, #FIBER_REG_X20]
    ldr x21, [x7, #FIBER_REG_X21]
    ldr x22, [x7, #FIBER_REG_X22]
    ldr x23, [x7, #FIBER_REG_X23]
    ldr x24, [x7, #FIBER_REG_X24]
    ldr x25, [x7, #FIBER_REG_X25]
    ldr x26, [x7, #FIBER_REG_X26]
    ldr x27, [x7, #FIBER_REG_X27]
    ldr x28, [x7, #FIBER_REG_X28]
    ldr x29, [x7, #FIBER_REG_X29]

    ldr d8,  [x7, #FIBER_REG_V8]
    ldr d9,  [x7, #FIBER_REG_V9]
    ldr d10, [x7, #FIBER_REG_V10]
    ldr d11, [x7, #FIBER_REG_V11]
    ldr d12, [x7, #FIBER_REG_V12]
    ldr d13, [x7, #FIBER_REG_V13]
    ldr d14, [x7, #FIBER_REG_V14]
    ldr d15, [x7, #FIBER_REG_V15]

    ldr x0, [x7, #FIBER_REG_X0]
    ldr x1, [x7, #FIBER_REG_X1]

    ldr x30, [x7, #FIBER_REG_X30]
    ldr x2,  [x7, #FIBER_REG_SP]
    mov sp, x2

    ret

#endif // defined(__aarch64__)