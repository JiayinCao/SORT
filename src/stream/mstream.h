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
 * IMemoryStream only works for streaming data from memory. Any attempt to write data
 * to memory will result in immediate crash.
 */
class IMemoryStream : public IStreamBase
{
public:
    //! @brief Streaming in a float number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (float& v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming in an integer number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (int& v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming in an unsigned integer number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (unsigned int& v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming in a string to memory.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (std::string& v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming in a boolean value to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (bool& v) override {
        // to be implemented
        return *this;
    }
};

//! @brief Streaming to memory.
/**
 * OMemoryStream only works for streaming data to memory. Any attempt to read data
 * from memory will result in immediate crash.
 */
class OMemoryStream : public OStreamBase
{
public:
    //! @brief Streaming out a float number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const float v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming out an integer number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const int v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming out an unsigned integer number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const unsigned int v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming out a string from memory.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const std::string& v) override {
        // to be implemented
        return *this;
    }

    //! @brief Streaming out a boolean value from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const bool v) override {
        // to be implemented
        return *this;
    }
};