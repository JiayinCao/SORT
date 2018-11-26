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

// include the header files
#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <math.h>

// find the correct platform
#if defined(_WIN32) || defined(_WIN64)
	#define SORT_IN_WINDOWS
#elif defined(__linux__)
	#define SORT_IN_LINUX
#elif defined(__APPLE__)
	#define SORT_IN_MAC
#endif

// enable debug by default
#define	SORT_DEBUG

#include <math.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1800) 
#  include <algorithm> // for std::min and std::max 
#endif

inline float LinearToGamma( float value ){
    if (value <= 0.0031308f) return 12.92f * value;
    return 1.055f * pow(value, (float)(1.f / 2.4f)) - 0.055f;
}
inline float GammaToLinear( float value ){
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return pow((value + 0.055f) * 1.f / 1.055f, (float)2.4f);
}
