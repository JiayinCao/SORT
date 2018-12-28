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

/*
    -------------------------- Coding Standard ------------------------------------
    SORT has been developed for quite a long time, several years. With limited 
    time budget everyday, progress is relatively slow. Throughout the past years, 
    my coding style changes, which results in different coding style in different 
    parts of the system. Following is some general rules to following during coding, 
    this is more of reminder for me for later references.

    -   Class name
        -   Each (sub)word in class name starts with a capital letter.
        -   Don't use names that are too long.
        -   Don't use meaningless names indicating nothing.
    -   Class method name
        -   Each (sub)word in class method names start with a capital letter.
        -   Don't use meaningless names indicating nothing.
    -   Class method parameter name
        -   Use small case letter for the first word. Starting from the second word,
            each word starts with a capital letter.
    -   Class member name
        -   Instance member variable should start with 'm_' to indicate it is an
            instance member.
        -   Static member variable should start with 's_' to indicate it is a
            static member.
        -   Use small case letter for the first word. Starting from the second word,
            each word starts with a capital letter.
    -   Class method modifiers
        -   Use minimal access range. Meaning if it is enough to use private, don't
            use protected to leak the access to derived class.
        -   Use const as much as possible. For example, for method like 'GetTaskID()'
            if nothing inside the function changes anything in the class, make sure 
            it is marked as const. It also applied to return values and parameters.
    -   Private method
        -   Same naming convention with public method apart from that the first letter
            of the first world is smaller case.
        -   Defined at last after member variable definition.
    -   Global method
        -   Same naming convention with private class method.
    -   Global parameters
        -   Start with 'g_'.
        -   First word has to be all small cases.
        -   Each of the first letter of the following words are upper cases.
    -   Brackets
        -   Open bracket should be appended right after class/method declaration or 
            definition to save line numbers.
        -   Closing bracket should occupy a separate line.
        -   For empty implementation, both the two brackets should be occupying the
            same line.
    -   Comment
        -   Doxygen standard. http://www.doxygen.nl/
    -   Overridden method
        -   All overridden class method has to specify 'override' for clarity. It is
            legit to not specify 'override'. However, with 'override' it is very
            clear if the function is an overridden function.
        -   Don't add virtual in an overridden function, which gives us nothing.
    -   Local Parameter type
        -   Use 'auto' as much as possible, instead of explicitly specific the type.

    ---------------------------------------------------------------------------------
*/ 

#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <math.h>

#if defined(_WIN32) || defined(_WIN64)
	#define SORT_IN_WINDOWS
#elif defined(__linux__)
	#define SORT_IN_LINUX
#elif defined(__APPLE__)
	#define SORT_IN_MAC
#endif

// enable debug by default
#define	SORT_DEBUG

#if defined(_MSC_VER) && (_MSC_VER >= 1800) 
#define NOMINMAX
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