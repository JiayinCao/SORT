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

// This file simply defines some macros to be shared by assembly language and cpp code

#define FIBER_REG_X0    0x00
#define FIBER_REG_X1    0x08
#define FIBER_REG_X16   0x10
#define FIBER_REG_X17   0x18
#define FIBER_REG_X18   0x20
#define FIBER_REG_X19   0x28
#define FIBER_REG_X20   0x30
#define FIBER_REG_X21   0x38
#define FIBER_REG_X22   0x40
#define FIBER_REG_X23   0x48
#define FIBER_REG_X24   0x50
#define FIBER_REG_X25   0x58
#define FIBER_REG_X26   0x60
#define FIBER_REG_X27   0x68
#define FIBER_REG_X28   0x70
#define FIBER_REG_X29   0x78
#define FIBER_REG_X30   0x80
#define FIBER_REG_V8    0x88
#define FIBER_REG_V9    0x90
#define FIBER_REG_V10   0x98
#define FIBER_REG_V11   0xa0
#define FIBER_REG_V12   0xa8
#define FIBER_REG_V13   0xb0
#define FIBER_REG_V14   0xb8
#define FIBER_REG_V15   0xc0
#define FIBER_REG_SP    0xc8