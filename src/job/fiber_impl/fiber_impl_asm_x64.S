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

#if defined(__x86_64__)

#include "fiber_impl_asm_x64_shared.h"

.text
.global ASM_ENTRY_WRAPPER(switch_fiber)
.align 4
ASM_ENTRY_WRAPPER(switch_fiber):

    // Save context 'from'

    // Store callee-preserved registers
    movq        %rbx, FIBER_REG_RBX(%rdi)
    movq        %rbp, FIBER_REG_RBP(%rdi)
    movq        %r12, FIBER_REG_R12(%rdi)
    movq        %r13, FIBER_REG_R13(%rdi)
    movq        %r14, FIBER_REG_R14(%rdi)
    movq        %r15, FIBER_REG_R15(%rdi)

    /* call stores the return address on the stack before jumping */
    movq        (%rsp), %rcx             
    movq        %rcx, FIBER_REG_RIP(%rdi)
    
    /* skip the pushed return address */
    leaq        8(%rsp), %rcx            
    movq        %rcx, FIBER_REG_RSP(%rdi)

    // Load context 'to'
    movq        %rsi, %r8

    // Load callee-preserved registers
    movq        FIBER_REG_RBX(%r8), %rbx
    movq        FIBER_REG_RBP(%r8), %rbp
    movq        FIBER_REG_R12(%r8), %r12
    movq        FIBER_REG_R13(%r8), %r13
    movq        FIBER_REG_R14(%r8), %r14
    movq        FIBER_REG_R15(%r8), %r15

    // Load first two call parameters
    movq        FIBER_REG_RDI(%r8), %rdi
    movq        FIBER_REG_RSI(%r8), %rsi

    // Load stack pointer
    movq        FIBER_REG_RSP(%r8), %rsp

    // Load instruction pointer, and jump
    movq        FIBER_REG_RIP(%r8), %rcx
    jmp         *%rcx

#endif // defined(__x86_64__)
