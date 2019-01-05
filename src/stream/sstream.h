/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.

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
#include "core/define.h"

//! @brief Streaming from socket through network.
/**
 * ISocketStream only works for streaming data from socket. Any attempt to write data
 * to socket will result in immediate crash.
 */
class ISocketStream : public IStreamBase{
public:
    //! @brief  Constructor.
    ISocketStream(){
    }

    //! @brief Streaming in a float number from socket.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (float& v) override {
        return *this;
    }

    //! @brief Streaming in an integer number from socket.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (int& v) override {
        return *this;
    }

    //! @brief Streaming in an unsigned integer number from socket.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (unsigned int& v) override {
        return *this;
    }

    //! @brief Streaming in a string from socket.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (std::string& v) override {
        return *this;
    }

    //! @brief Streaming in a boolean value from socket.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (bool& v) override {
        return *this;
    }

    //! @brief Loading data from stream directly.
    //!
    //! @param  data    Data to be filled.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Load( char* data , int size ) override {
        return *this;
    }
};

//! @brief Streaming to socket.
/**
 * OSocketStream only works for streaming data to socket. Any attempt to read data
 * from socket will result in immediate crash.
 */
class OSocketStream : public OStreamBase{
public:
    //! @brief  Constructor.
    OSocketStream(){
    }

    //! @brief Streaming in a float number from socket.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const float v) override {
        return *this;
    }

    //! @brief Streaming in an integer number from socket.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const int v) override {
        return *this;
    }

    //! @brief Streaming in an unsigned integer number from socket.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const unsigned int v) override {
        return *this;
    }

    //! @brief Streaming in a string from socket.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const std::string& v) override {
        return *this;
    }

    //! @brief Streaming in a boolean value from socket.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const bool v) override {
        return *this;
    }

    //! @brief Writing data to stream.
    //!
    //! @param  data    Data to be written.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Write( char* data , int size ) override {
        return *this;
    }
};