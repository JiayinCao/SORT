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

// This file simply defines some macros to be shared by assembly language and C code

#include "core/define.h"

#define FIBER_REG_RBX 0x00
#define FIBER_REG_RBP 0x08
#define FIBER_REG_R12 0x10
#define FIBER_REG_R13 0x18
#define FIBER_REG_R14 0x20
#define FIBER_REG_R15 0x28
#define FIBER_REG_RDI 0x30
#define FIBER_REG_RSI 0x38
#define FIBER_REG_RSP 0x40
#define FIBER_REG_RIP 0x48

#if defined(SORT_IN_MAC)
    #define ASM_ENTRY_WRAPPER(x) _##x
#else
    #define ASM_ENTRY_WRAPPER(x) x
#endif