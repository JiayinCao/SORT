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

//! @brief  Fiber context.
/**
 * Differnet from the assembly implementation, thanks to the OS provided fiber interface,
 * I only really need to call the interface to implement fiber on Windows.
 * Technically speaking, I could have used assembly language implementation like I did
 * on other platforms. However, system provided interface is a lot easier for me and I 
 * could rely on Windows interface to offer a much more robust implementation as well.
 * 
 * In the future, if there is something that is wrong and seems to be caused by low level
 * fiber implementation, I can tell if this is the case by checking Windows build, which
 * could serve as a reference implementation.
 */
struct FiberContext{
    // handle to hold fiber
    LPVOID  m_fiber = nullptr;

    // whether the fiber is originated from a thread or not
    bool    m_isFiberFromThread = false;
};

#endif