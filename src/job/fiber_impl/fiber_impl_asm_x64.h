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

#include "fiber_impl_asm_x64_shared.h"

// type of each register, has to be 64 bits
typedef unsigned long long  Register;

//! @brief  Fiber context that saves all the callee saved registers
/**
 * The specific set of register is architecture and OS dependent. SORT uses this implementation for
 * Intel Mac and X64 Ubuntu, which use 'System V AMD64 ABI'.
 * https://en.wikipedia.org/wiki/X86_calling_conventions#cite_note-AMD-28
 * 
 * System V Application Binary Interface AMD64 Architecture Processor Supplement
 * Page 23, AMD64 ABI Draft 1.0
 * https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-1.0.pdf
 */
struct FiberContext {
    // callee-saved registers
    Register rbx;
    Register rbp;
    Register r12;
    Register r13;
    Register r14;
    Register r15;

    // parameter registers
    Register rdi;
    Register rsi;

    // stack and instruction registers
    Register rsp;
    Register rip;
};

#if defined(__cplusplus)

#include <cstddef>

// X64's register should be 64 bits wide
static_assert(sizeof(Register) == 8);

#define CHECK_OFFSET(REGISTER, OFFSET)    \
    static_assert(offsetof(FiberContext, REGISTER) == OFFSET,      "Incorrect register offset")

CHECK_OFFSET(rbx, FIBER_REG_RBX);
CHECK_OFFSET(rbp, FIBER_REG_RBP);
CHECK_OFFSET(r12, FIBER_REG_R12);
CHECK_OFFSET(r13, FIBER_REG_R13);
CHECK_OFFSET(r14, FIBER_REG_R14);
CHECK_OFFSET(r15, FIBER_REG_R15);
CHECK_OFFSET(rdi, FIBER_REG_RDI);
CHECK_OFFSET(rsi, FIBER_REG_RSI);
CHECK_OFFSET(rsp, FIBER_REG_RSP);
CHECK_OFFSET(rip, FIBER_REG_RIP);

extern "C" {
    // @brief   Low level function to switch fibers, this is implemented in assembly language
    extern void switch_fiber(FiberContext* from, const FiberContext* to);

    // @brief   Create a brand new fiber
    extern void create_fiber(FiberContext* context, void* stack, uint32_t stackSize, void (*target)(void*), void* arg);
}

#endif