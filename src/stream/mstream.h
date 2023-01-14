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

#include <cstring>
#include "stream.h"
#include "core/define.h"

//! @brief Streaming from memory.
/**
 * IMemoryStream only works for streaming data from memory. Any attempt to write data
 * to memory will result in immediate crash.
 */
class OMemoryStream : public OStreamBase {
public:
    //! @brief  Constructor.
    //!
    //! @param  initSize    Initial size of the stream.
    OMemoryStream(unsigned initSize = 1024u) {
        if (initSize == 0u)
            return;
        Resize(initSize);
    }

    //! @brief  Resize the stream.
    //!
    //! @param  size    The new size to be resized.
    SORT_FORCEINLINE void    Resize(unsigned int size) {
        size = std::max(1024u, size);
        auto new_data = std::make_unique<char[]>(size);
        if (m_capacity)
            memcpy(new_data.get(), m_data.get(), m_capacity);
        if (size > m_capacity)
            memset((char*)new_data.get() + m_capacity, 0, size - m_capacity);
        m_capacity = size;
        m_data = std::move(new_data);
    }

    //! @brief Streaming in a float number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const float v) override {
        if (m_pos + sizeof(v) > m_capacity) {
            Resize(2 * m_capacity);
            return *this << v;
        }
        else {
            memcpy(m_data.get() + m_pos, &v, sizeof(v));
            m_pos += sizeof(v);
        }
        return *this;
    }

    //! @brief Streaming in an integer number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const int v) override {
        if (m_pos + sizeof(v) > m_capacity) {
            Resize(2 * m_capacity);
            return *this << v;
        }
        else {
            memcpy(m_data.get() + m_pos, &v, sizeof(v));
            m_pos += sizeof(v);
        }
        return *this;
    }

    //! @brief Streaming in an 8 bit integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const char v) override {
        if (m_pos + sizeof(v) > m_capacity) {
            Resize(2 * m_capacity);
            return *this << v;
        }
        else {
            memcpy(m_data.get() + m_pos, &v, sizeof(v));
            m_pos += sizeof(v);
        }
        return *this;
    }

    //! @brief Streaming in an unsigned integer number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const unsigned int v) override {
        if (m_pos + sizeof(v) > m_capacity) {
            Resize(2 * m_capacity);
            return *this << v;
        }
        else {
            memcpy(m_data.get() + m_pos, &v, sizeof(v));
            m_pos += sizeof(v);
        }
        return *this;
    }

    //! @brief Streaming in a string from memory.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const std::string& v) override {
        if ((v.size() + 1) + m_pos > m_capacity) {
            Resize(2 * m_capacity);
            return *this << v;
        }

        if (!v.empty()) {
            memcpy(m_data.get() + m_pos, v.c_str(), sizeof(char) * v.size());
            m_pos += (unsigned int)v.size();
        }
        m_data[m_pos++] = 0;
        return *this;
    }

    //! @brief Streaming in a boolean value from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const bool v) override {
        if (m_pos + sizeof(v) > m_capacity) {
            Resize(2 * m_capacity);
            return *this << v;
        }
        else {
            memcpy(m_data.get() + m_pos, &v, sizeof(v));
            m_pos += sizeof(v);
        }
        return *this;
    }

    //! @brief  Get written size of the stream.
    //!
    //! @return     The size of written bytes.
    unsigned int    GetDataSize() const {
        return m_pos;
    }

    //! @brief Reading data from stream.
    //!
    //! @param  data    Data to be filled.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Write(char* data, int size) override {
        if (m_pos + size > m_capacity) {
            Resize(2 * m_capacity);
            return Write(data, size);
        }
        else {
            memcpy(m_data.get() + m_pos, data, size);
            m_pos += size;
        }
        return *this;
    }

    //! @brief  Get raw data;
    const char* GetData() const {
        return m_data.get();
    }

    //! @brief  Clear the data
    void Clear() {
        m_pos = 0;
    }

    //! @brief Get current position. Ideally, IStreamBase might need to have this interface too. But it is not currently needed.
    unsigned    GetPos() override {
        return m_pos;
    }

    //! @brief Seek to a specific position
    void        Seek(unsigned int p) override {
        m_pos = p;
    }

private:
    /**< Pointer points to the address where the memory is. */
    std::unique_ptr<char[]>     m_data = nullptr;
    /**< Capacity of the current stream. */
    unsigned int                m_capacity = 0;
    /**< Current position of streaming. */
    unsigned int                m_pos = 0;
};

//! @brief Streaming to memory.
/**
 * OMemoryStream only works for streaming data to memory. Any attempt to read data
 * from memory will result in immediate crash.
 */
class IMemoryStream : public IStreamBase{
public:
    //! @brief  Constructor.
    //!
    //! @param  istream      The IMemoryStream as input.
    IMemoryStream(const char* data, unsigned int initSize){
        Resize(initSize);
        memcpy(m_data.get(), data, initSize);
    }

    //! @brief  Resize the stream.
    //!
    //! @param  size    The new size to be resized.
    SORT_FORCEINLINE void    Resize( unsigned int size ){
        size = std::max( 1024u , size );
        auto new_data = std::make_unique<char[]>( size );
        if( m_capacity )
            memcpy( new_data.get() , m_data.get() , m_capacity );
        if (size > m_capacity)
            memset(new_data.get() + m_capacity, 0, size - m_capacity);
        m_capacity = size;
        m_data = std::move(new_data);
    }

    //! @brief Streaming out a float number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (float& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data.get() + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming out an integer number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (int& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data.get() + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming out an 8 bit integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (char& v) override {
        if (m_pos + sizeof(v) > m_capacity)
            v = 0;
        else {
            memcpy(&v, m_data.get() + m_pos, sizeof(v));
            m_pos += sizeof(v);
        }
        return *this;
    }

    //! @brief Streaming out an unsigned integer number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (unsigned int& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data.get() + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming out a string to memory.
    //!
    //! Unlike standard stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (std::string& v) override {
        v = "";
        while( m_pos < m_capacity ){
            char c = m_data[m_pos++];
            if( c == 0 )
                break;
            v += c;
        }
        return *this;
    }

    //! @brief Streaming out a boolean value to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (bool& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data.get() + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Writing data to stream directly.
    //!
    //! @param  data    Data to be filled.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Load( char* data , int size ) override {
        if( m_pos + size > m_capacity ){
            memset( data , 0 , size );
        }else{
            memcpy( data , m_data.get() , size );
            m_pos += size;
        }
        return *this;
    }

private:
    /**< Pointer points to the address where the memory is. */
    std::unique_ptr<char[]>     m_data = nullptr;
    /**< Capacity of the current stream. */
    unsigned int                m_capacity = 0;
    /**< Current position of streaming. */
    unsigned int                m_pos = 0;
};