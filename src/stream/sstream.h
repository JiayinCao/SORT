/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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

#ifdef SORT_IN_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#undef NOMINMAX
#endif

#include "stream.h"
#include "core/define.h"
#include "core/socket.h"
#include "stream/mstream.h"

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
    
    //! @brief Streaming out an 8 bit integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (char& v) override {
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
 * By default, writting data to the stream won't trigger a flush. It has to be explictly called.
 */
class OSocketStream : public OMemoryStream {
public:
    //! @brief  Constructor.
    OSocketStream(socket_t socket):m_socket(socket){}

    //! @brief Flush current written result.
    void Flush() override{
        // send the data out
        auto size = GetDataSize();
        auto data = GetData();
        while( size > 0 ){
            const auto size_to_send = std::min(SEND_MAX_SIZE, size);
#ifdef SORT_IN_WINDOWS
            const auto flag = 0;
#else
            // don't panic if we lost socket connection, otherwise this will lead to crash.
            const auto flag = MSG_NOSIGNAL;
#endif
            const auto byte_sent = send(m_socket, data, size_to_send, flag);

            // it is possible that the socket is disconnected already.
            if( byte_sent < 0 )
                break;

            size -= byte_sent;
            data += byte_sent;
        }
        Clear();
    }

private:
    socket_t m_socket;

    static constexpr unsigned SEND_MAX_SIZE = 4096;
};