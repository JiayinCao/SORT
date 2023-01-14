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

#pragma once

#include "core/define.h"

// SORT used to use easy profiler as default one.
// It is a open-source cross-platform project, which is available on git https://github.com/yse/easy_profiler

#ifdef SORT_ENABLE_PROFILER

#include "easy/profiler.h"

#define SORT_PROFILE_ENABLE         EASY_PROFILER_ENABLE
#define SORT_PROFILE_DISABLE        EASY_PROFILER_DISABLE
#define SORT_PROFILE_ISENABLED      ::profiler::isEnabled()
#define SORT_PROFILE(e)             EASY_BLOCK((e))
#define SORT_PROFILE_END            EASY_END_BLOCK
#define SORT_PROFILE_DUMP(file)     profiler::dumpBlocksToFile(file)

#else

#define SORT_PROFILE_ENABLE         {}
#define SORT_PROFILE_DISABLE        {}
#define SORT_PROFILE_ISENABLED      false
#define SORT_PROFILE(e)
#define SORT_PROFILE_END
#define SORT_PROFILE_DUMP(file)     0

#endif
