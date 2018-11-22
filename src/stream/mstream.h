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

#include "stream.h"

//! @brief Streaming from memory.
/**
 * IMemoryStream only works for stream data from memory. Any attempt to write data
 * to memory will result in immediate crash.
 */
class IMemoryStream : public IStream
{
public:
    //! @brief Streaming in a float number to memory
    //!
    //! @param t    Value to be loaded
    void operator >> (float& t) override {
    }
};

//! @brief Streaming to memory.
/**
 * OMemoryStream only works for stream data to memory. Any attempt to read data
 * from memory will result in immediate crash.
 */
class OMemoryStream : public OStream
{
public:
    //! @brief Streaming out a float number from memory
    //!
    //! @param t    Value to be saved
    void operator << (const float t) override {
    }
};