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

#include "core/sassert.h"
#include "core/log.h"
#include "math/point.h"
#include "math/vector2.h"
#include "math/transform.h"
#include "spectrum/spectrum.h"
#include "core/strid.h"

//! @brief Interface for streaming/serialization.
/**
 * StreamBase is an interface defining the basic feature of serialization. It is an abstract class
 * that doesn't allow to be instanced. The pure virtual functions defines the minimal interface
 * supporting the streaming. The destination of where to stream is dependent on the derived class.
 * It could go to file by using FileStream, it could also flow to memory by using MemoryStream.
 * There is no guarantee that StreamBase is perfectly thread safe, it is the user's responsibility to
 * avoid it.
 */
class StreamBase{
public:
    //! @brief  Empty virtual destructor.
    virtual ~StreamBase() {}

    //! @brief Streaming in a float number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator << (const float v) = 0;

    //! @brief Streaming out a float number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator >> (float& v) = 0;

    //! @brief Streaming in a integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator << (const int v) = 0;

    //! @brief Streaming out a integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator >> (int& v) = 0;

    //! @brief Streaming in an 8 bit integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator << (const char v) = 0;

    //! @brief Streaming out an 8 bit integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator >> (char& v) = 0;

    //! @brief Streaming in an unsigned integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator << (const unsigned int v) = 0;

    //! @brief Streaming out an unsigned integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator >> (unsigned int& v) = 0;

    //! @brief Streaming in a string.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator << (const std::string& v) = 0;

    //! @brief Streaming out a string
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator >> (std::string& v) = 0;

    //! @brief Streaming in a boolean value.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator << (const bool v) = 0;

    //! @brief Streaming out a boolean value.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual StreamBase& operator >> (bool& v) = 0;

    //! @brief Streaming in a point value
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator << ( const Point& v ){
        return *this << v.x << v.y << v.z;
    }

    //! @brief Streaming out a point value
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> ( Point& v ){
        return *this >> v.x >> v.y >> v.z;
    }

    //! @brief Streaming in a vector value
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator << ( const Vector& v ){
        return *this << v.x << v.y << v.z;
    }

    //! @brief Streaming out a vector value
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> ( Vector& v ){
        return *this >> v.x >> v.y >> v.z;
    }

    //! @brief Streaming in a vector value
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator << (const Vector2f& v) {
        return *this << v.x << v.y;
    }

    //! @brief Streaming out a vector value
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> (Vector2f& v) {
        return *this >> v.x >> v.y;
    }

    //! @brief Streaming in a transform value.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator << (const Transform& v) {
        return *this << v.matrix;
    }

    //! @brief Streaming out a transform value.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> (Transform& v) {
        *this >> v.matrix;
        v.matrix.Inverse(v.invMatrix);
        return *this;
    }

    //! @brief Streaming in a matrix value.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator << (const Matrix& v) {
        for (int i = 0; i < 16; ++i)
            *this << v.m[i];
        return *this;
    }

    //! @brief Streaming out a matrix value.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> (Matrix& v) {
        for (int i = 0; i < 16; ++i)
            *this >> v.m[i];
        return *this;
    }

    //! @brief Streaming in a spectrum value.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator << (const Spectrum& v) {
        *this << v.r << v.g << v.b;
        return *this;
    }

    //! @brief Streaming out a spectrum value.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> (Spectrum& v) {
        float r , g , b;
        *this >> r >> g >> b;
        v = Spectrum ( r , g , b );
        return *this;
    }

    //! @brief  Streaming in a StringID
    //!
    //! @param sid  Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator <<( const StringID sid ){
        return *this << sid.m_sid;
    }

    //! @brief Streaming out a StringID.
    //!
    //! @param sid  Value to be loaded.
    //! @return     Reference of the stream itself.
    SORT_FORCEINLINE StreamBase&  operator >> (StringID& sid) {
        return *this >> sid.m_sid;
    }

    //! @brief Loading data from stream directly.
    //!
    //! @param  data    Data to be filled.
    //! @param  size    Size of the data to be filled in bytes.
    virtual StreamBase& Load( char* data , int size ) = 0;

    //! @brief Writing data to stream.
    //!
    //! @param  data    Data to be written.
    //! @param  size    Size of the data to be filled in bytes.
    virtual StreamBase& Write( char* data , int size ) = 0;
};

//! @brief Streaming in data
/**
 * IStreamBase is purely for streaming in data. Trying to stream out data with IStreamBase will result in
 * crash. It is higher level code's responsibility to make sure it won't happen.
 */
class IStreamBase : public StreamBase {
public:
    //! @brief Disable streaming in a float number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const float v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }

    //! @brief Disable streaming in a integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const int v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }

    //! @brief Streaming in an 8 bit integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const char v) final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }

    //! @brief Disable streaming in an unsigned integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const unsigned int v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }

    //! @brief Disable streaming in a string. Attempting to do it will result in crash!
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const std::string& v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }

    //! @brief Disable streaming in a boolean value. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (const bool v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }

    //! @brief Writing data to stream.
    //!
    //! @param  data    Data to be written.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Write( char* data , int size ) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStreamBase!"); return *this; }
};

//! @brief Streaming out data
/**
 * OStreamBase is purely for streaming in data. Trying to stream in data with OStreamBase will result in
 * crash. It is higher level code's responsibility to make sure it won't happen.
 */
class OStreamBase : public StreamBase{
public:
    //! @brief Disable streaming out a float number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (float& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStreamBase!"); return *this; }

    //! @brief Disable streaming out a integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (int& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStreamBase!"); return *this; }

    //! @brief Streaming out an 8 bit integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (char& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStreamBase!"); return *this; }

    //! @brief Disable streaming out an unsigned integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (unsigned int& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStreamBase!"); return *this; }

    //! @brief Disable streaming out a string. Attempting to do it will result in crash!
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (std::string& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStreamBase!"); return *this; }

    //! @brief Disable streaming out a boolean value. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (bool& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }

    //! @brief Disable streaming in an array of bytes. Attempting to do it will result in crash!
    //!
    //! @param  data    Data to be filled.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Load( char* data , int size ) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }

    //! @brief Flush current written result.
    virtual void Flush() {}

    //! @brief Get current position. Ideally, IStreamBase might need to have this interface too. But it is not currently needed.
    virtual unsigned    GetPos() = 0;

    //! @brief Seek to a specific position
    virtual void        Seek(unsigned int) = 0;
};

//! @brief  Interface for serializable objects.
//!
//! Any derived class will need to implement the two interface supporting serialization of objects.
class SerializableObject{
public:
    //! @brief      Empty destructor
    virtual ~SerializableObject() {}

    //! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    virtual void    Serialize( IStreamBase& stream ) = 0;

    //! @brief      Serializing data to stream
    //!
    //! Unlike serializing data from stream, this version has a default empty implementation. SORT won't implement serializing its scene into stream
    //! for simplicity. There is almost no occasion when it is needed. It is the plugin's task to stream out the scene.
    //!
    //! @param      Stream where the serialization data goes to. Depending on different situation, it could come from different places.#pragma endregion
    virtual void    Serialize( OStreamBase& stream ) {};
};