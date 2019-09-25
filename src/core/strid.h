/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

// Cyclic redundancy check
// https://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRC-32_algorithm
//
// Computation of cyclic redundancy checks
// https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
//
// Fast CRC32
// https://create.stephan-brumme.com/crc32/#bitwise

constexpr unsigned int crc32_bitwise(const char* data, std::size_t length ){
    constexpr unsigned int Polynomial = 0xEDB88320;

    unsigned int crc = 0;
    while (length--){
        crc ^= *data++;

        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
    }
    return ~crc;
}

// Game Engine Architecture, Chapter 6.4.3.2
// https://www.gameenginebook.com/
// 
// String will be encoded as a 32-bit integer in SORT.

using StringID = unsigned int;

// Appending '_sid' converts a string to StringID, which is essentially an unsigned integer.
constexpr StringID operator"" _sid( const char* str , std::size_t len ){
    return crc32_bitwise( str , len );
}

// A helper macro to convert string to StringID
#define SID(str)                str##_sid

// A helper macro to convert stringid to unsigned int, this is to hide detail of StringID so that
// other parts of SORT won't assume that StringID is an unsigned integer.
#define UINT_FROM_STRINGID(id)  id  