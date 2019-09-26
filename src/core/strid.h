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

// Current type of stringID is actually a 32 bits integer, which should be good enough for now.
using sid_t = unsigned int;

// Use 0 as invalid string id, hopefully nothing but empty string ends up with the value of 0xFFFFFFFF.
#define INVALID_SID   sid_t(0xFFFFFFFF)

// Cyclic redundancy check
// https://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRC-32_algorithm
//
// Computation of cyclic redundancy checks
// https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
//
// Fast CRC32
// https://create.stephan-brumme.com/crc32/#bitwise

constexpr unsigned int Polynomial = 0xEDB88320;

constexpr unsigned int crc32_bitwise(const char* data, std::size_t length ){
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

// Intern string
inline sid_t InternString( const char* str ){
    return crc32_bitwise( str , std::strlen( str ) );
}

// Appending '_sid' converts a string to StringID, which is essentially an unsigned integer.
constexpr sid_t operator"" _sid( const char* str , std::size_t len ){
    return (sid_t)(crc32_bitwise( str , len ));
}

// A helper macro to convert string to StringID
#define SID(str)                str##_sid

/**
 * Game Engine Architecture, Chapter 6.4.3.2
 * https://www.gameenginebook.com/
 * 
 * String will be encoded as a 32-bit integer in SORT. The encoded integer is hiden in this class
 * named StringID.
 */
struct StringID{
    //! @brief Constructor taking in an unsigned integer.
    //!
    //! @param sid   The encoded hash id of the original string.
    StringID( sid_t sid = INVALID_SID ) : m_sid(sid) {}

    //! @brief Constructor taking in a string.
    //!
    //! @param str   The string to be encoded.
    StringID( const char* str ) : m_sid( InternString(str) ) {}

    //! @brief Copy constructor
    //!
    //! @param other  The source to copy from.
    StringID( const StringID& other ) : m_sid( other.m_sid ) {}

    //! @brief = operator
    //!
    //! @param other  The source to copy from.
    const StringID& operator = ( const StringID& other ) {
        m_sid = other.m_sid;
        return *this;
    }

    //! @brief == operator
    //!
    //! @param other  Comparing with the other StringID.
    bool operator == ( const StringID& other ) const {
        return m_sid == other.m_sid;
    }

    //! @brief != operator
    //!
    //! @param other  Comparing with the other StringID.
    bool operator != ( const StringID& other ) const {
        return m_sid != other.m_sid;
    }

    sid_t   m_sid;  /**< Hashed id of the original string. */
};

// Use the CRC hashed key as key in hash table directly, this may not result in the best performance
// due to the simplicity of the hash function, but it is simpler.
template <>
struct std::hash<StringID>{
    std::size_t operator()(const StringID& key ) const{
        return (std::size_t)key.m_sid;
    }
};

// Global helper function for comparing StringID with sid
// StringID is passed by value instead of reference is on purpose.
inline bool operator == ( const sid_t other , const StringID sid ) {
    return sid.m_sid == other;
}
inline bool operator == ( const StringID sid , const sid_t other ) {
    return sid.m_sid == other;
}

// Make sure the size of StringID is no larger than its internal represenation.
static_assert( sizeof( StringID ) == sizeof( sid_t ) , "StringID size is incorrect, it should be 32bits.");