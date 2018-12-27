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
#include "core/define.h"
#include "stream/fstream.h"

//! @brief Streaming from memory.
/**
 * IMemoryStream only works for streaming data from memory. Any attempt to write data
 * to memory will result in immediate crash.
 */
class IMemoryStream : public IStreamBase{
public:
    //! @brief  Constructor.
    //!
    //! It is the caller's job to make sure the memory passed in has at least 'initSize'
    //! bytes available. There will be unknown behavior otherwise.
    //!
    //! @param  initSize    Initial size of the stream.
    //! @param  initData    Initial data of the stream.
    IMemoryStream( unsigned int initSize = 1024 , void* initData = nullptr ){
        Resize( initSize );
        memcpy( m_data , initData , initSize );
    }

    //! @bried  Destructor.
    ~IMemoryStream(){
        SAFE_DELETE_ARRAY( m_data );
    }

    //! @brief  Resize the stream.
    //!
    //! @param  size    The new size to be resized.
    inline void    Resize( unsigned int size ){
        m_data = (char *) ( ( m_data == NULL ) ? malloc(size) : realloc(m_data, size) );
        if (size > m_capacity)
            memset((char*)m_data + m_capacity, 0, size - m_capacity);
	    m_capacity = size;
    }

    //! @brief Streaming in a float number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (float& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming in an integer number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (int& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming in an unsigned integer number to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (unsigned int& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
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
        v = "";
        while( m_pos < m_capacity ){
            char c = m_data[m_pos++];
            if( c == 0 )
                break;
            v += c;
        }
        return *this;
    }

    //! @brief Streaming in a boolean value to memory.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (bool& v) override {
        if( m_pos + sizeof( v ) > m_capacity )
            v = 0;
        else{
            memcpy( &v , m_data + m_pos , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief  Get raw memory.
    //!
    //! The caller doesn't own the memory. It has to make sure its life time still expand its usage.
    //!
    //! @return     Return a pointer points to the memory.
    void*       GetRawData(){
        return m_data;
    }

private:
    /**< Pointer points to the address where the memory is. */
    char*               m_data = nullptr;
    /**< Capacity of the current stream. */
    unsigned int        m_capacity = 0;
    /**< Current position of streaming. */
    unsigned int        m_pos = 0;
};

//! @brief Streaming to memory.
/**
 * OMemoryStream only works for streaming data to memory. Any attempt to read data
 * from memory will result in immediate crash.
 */
class OMemoryStream : public OStreamBase{
public:
    //! @brief  Constructor.
    //!
    //! It is the caller's job to make sure the memory passed in has at least 'initSize'
    //! bytes available. There will be unknown behavior otherwise.
    //!
    //! @param  initSize    Initial size of the stream.
    OMemoryStream( unsigned int initSize = 1024 ){
        Resize( initSize );
    }

    //! @bried  Destructor.
    ~OMemoryStream(){
        SAFE_DELETE_ARRAY( m_data );
    }

    //! @brief  Resize the stream.
    //!
    //! @param  size    The new size to be resized.
    inline void    Resize( unsigned int size ){
        m_data = (char *) ( ( m_data == NULL ) ? malloc(size) : realloc(m_data, size) );
        if (size > m_capacity)
            memset((char*)m_data + m_capacity, 0, size - m_capacity);
	    m_capacity = size;
    }

    //! @brief Streaming out a float number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const float v) override {
        if( m_pos + sizeof( v ) > m_capacity ){
            Resize( 2 * m_capacity );
            return *this << v;
        }else{
            memcpy( m_data + m_pos , &v , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming out an integer number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const int v) override {
        if( m_pos + sizeof( v ) > m_capacity ){
            Resize( 2 * m_capacity );
            return *this << v;
        }else{
            memcpy( m_data + m_pos , &v , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Streaming out an unsigned integer number from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const unsigned int v) override {
        if( m_pos + sizeof( v ) > m_capacity ){
            Resize( 2 * m_capacity );
            return *this << v;
        }else{
            memcpy( m_data + m_pos , &v , sizeof( v ) );
            m_pos += sizeof( v );
        }
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
        if( !v.empty() ){
            memcpy( m_data + m_pos , v.c_str() , sizeof(char) * v.size() );
            m_pos += (unsigned int)v.size();
        }
        m_data[m_pos++] = 0;
        return *this;
    }

    //! @brief Streaming out a boolean value from memory.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const bool v) override {
        if( m_pos + sizeof( v ) > m_capacity ){
            Resize( 2 * m_capacity );
            return *this << v;
        }else{
            memcpy( m_data + m_pos , &v , sizeof( v ) );
            m_pos += sizeof( v );
        }
        return *this;
    }

    //! @brief Get raw memory.
    //!
    //! The caller doesn't own the memory. It has to make sure its life time still expand its usage.
    //!
    //! @return     Return a pointer points to the memory.
    void*           GetRawData(){
        return m_data;
    }

    //! @brief  Get written size of the stream.
    //!
    //! @return     The size of written bytes.
    unsigned int    GetDataSize() const { 
        return m_pos;
    }

private:
    /**< Pointer points to the address where the memory is. */
    char*               m_data = nullptr;
    /**< Capacity of the current stream. */
    unsigned int        m_capacity = 0;
    /**< Current position of streaming. */
    unsigned int        m_pos = 0;
};
