/*
This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
platform physically based renderer.

Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.

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

#include "../sort.h"

// SORT used to use this profiler as default one.
// It is a open-source cross-platform project, which is available on git https://github.com/yse/easy_profiler
// However, it doesn't scale well with billions of rays, for which reason, this feature is dropped.

// Seeking more affordable implementation. And the task-based graph is not as useful as it is in real time rendering
// because it is all very similar task for every ray. A random sampling profiler may be more suitable for this project.
// Here is the Github issue tracking the feature
// https://github.com/JerryCao1985/SORT/issues/69

#ifdef SORT_ENABLE_PROFILER

// There is no implementation yet
#define SORT_PROFILE_ENABLE
#define SORT_PROFILE_ISENABLED      false
#define SORT_PROFILE(e)
#define SORT_PROFILE_END
#define SORT_PROFILE_DUMP(file)     0

#else

#define SORT_PROFILE_ENABLE
#define SORT_PROFILE_ISENABLED      false
#define SORT_PROFILE(e)
#define SORT_PROFILE_END
#define SORT_PROFILE_DUMP(file)     0

#endif