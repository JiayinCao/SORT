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

#include "fiber_impl_asm_aarch64_shared.h"

// type of each register, has to be 64 bits
typedef unsigned long long  Register;

//! @brief  Fiber context that saves all the callee saved registers
/**
 * On Arm 64 architecture, each register should be 64 bits. And following registers need to be
 * saved during a context switch
 *   - [X16 - X30]
 *     https://developer.arm.com/documentation/102374/0101/Procedure-Call-Standard
 *     By the time this code is written, there is an error on the bottom of the page about float point registers.
 *     Just ignore it if it is still wrong.
 *   - [D16 - D23]
 *     https://developer.arm.com/documentation/den0024/a/The-ABI-for-ARM-64-bit-Architecture/Register-use-in-the-AArch64-Procedure-Call-Standard/Parameters-in-NEON-and-floating-point-registers
 */
struct FiberContext {
    // funciton paramters
    Register    x0;
    Register    x1;

    // general purpose registers
    Register    x16;    // IP0
    Register    x17;    // IP1
    Register    x18;
    Register    x19;
    Register    x20;
    Register    x21;
    Register    x22;
    Register    x23;
    Register    x24;
    Register    x25;
    Register    x26;
    Register    x27;
    Register    x28;
    Register    x29;    // frame pointer (FP)
    Register    x30;    // link register (R30)

    // float-point and neon registers
    Register    v8;
    Register    v9;
    Register    v10;
    Register    v11;
    Register    v12;
    Register    v13;
    Register    v14;
    Register    v15;

    // stack pointer
    Register    sp;     
};

#if defined(__cplusplus)

#include <cstddef>

// Arm 64's register should be 64 bits wide
static_assert(sizeof(Register) == 8);

#define CHECK_OFFSET(REGISTER, OFFSET)    \
    static_assert(offsetof(FiberContext, REGISTER) == OFFSET,      "Incorrect register offset")

// Making sure our offsets are all correct
CHECK_OFFSET(x0,  FIBER_REG_X0);
CHECK_OFFSET(x1,  FIBER_REG_X1);
CHECK_OFFSET(x16, FIBER_REG_X16);
CHECK_OFFSET(x17, FIBER_REG_X17);
CHECK_OFFSET(x18, FIBER_REG_X18);
CHECK_OFFSET(x19, FIBER_REG_X19);
CHECK_OFFSET(x20, FIBER_REG_X20);
CHECK_OFFSET(x21, FIBER_REG_X21);
CHECK_OFFSET(x22, FIBER_REG_X22);
CHECK_OFFSET(x23, FIBER_REG_X23);
CHECK_OFFSET(x24, FIBER_REG_X24);
CHECK_OFFSET(x25, FIBER_REG_X25);
CHECK_OFFSET(x26, FIBER_REG_X26);
CHECK_OFFSET(x27, FIBER_REG_X27);
CHECK_OFFSET(x28, FIBER_REG_X28);
CHECK_OFFSET(x29, FIBER_REG_X29);
CHECK_OFFSET(x30, FIBER_REG_X30);
CHECK_OFFSET(v8,  FIBER_REG_V8);
CHECK_OFFSET(v9,  FIBER_REG_V9);
CHECK_OFFSET(v10, FIBER_REG_V10);
CHECK_OFFSET(v11, FIBER_REG_V11);
CHECK_OFFSET(v12, FIBER_REG_V12);
CHECK_OFFSET(v13, FIBER_REG_V13);
CHECK_OFFSET(v14, FIBER_REG_V14);
CHECK_OFFSET(v15, FIBER_REG_V15);
CHECK_OFFSET(sp,  FIBER_REG_SP);

extern "C" {
    // @brief   Low level function to switch fibers, this is implemented in assembly language
    extern void switch_fiber(FiberContext* from, const FiberContext* to);

    // @brief   Create a brand new fiber
    extern void create_fiber(FiberContext* context, void* stack, uint32_t stackSize, void (*target)(void*), void* arg);
}

#endif