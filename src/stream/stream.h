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

#include "utility/sassert.h"
#include "utility/log.h"

//! @brief Interface for streaming/serialization.
/**
 * Stream is an interface defining the basic feature of serialization. It is an abstract class
 * that doesn't allow to be instanced. The pure virtual functions defines the minimal interface
 * supporting the streaming. The destination of where to stream is dependent on the derived class.
 * It could go to file by using FileStream, it could also flow to memory by using MemoryStream.
 * There is no guarantee that Stream is perfectly thread safe, it is the user's responsibility to
 * avoid it.
 */
class Stream 
{
public:
    //! Empty virtual destructor.
    virtual ~Stream() {}

    //! @brief Streaming in a float number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual Stream& operator << (const float v) = 0;

    //! @brief Streaming out a float number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual Stream& operator >> (float& v) = 0;

    //! @brief Streaming in a integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual Stream& operator << (const int v) = 0;

    //! @brief Streaming out a integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual Stream& operator >> (int& v) = 0;

    //! @brief Streaming in an unsigned integer number.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual Stream& operator << (const unsigned int v) = 0;

    //! @brief Streaming out an unsigned integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual Stream& operator >> (unsigned int& v) = 0;

    //! @brief Streaming in a string.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual Stream& operator << (const std::string& v) = 0;

    //! @brief Streaming out a string
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual Stream& operator >> (std::string& v) = 0;

    //! @brief Streaming in a boolean value.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    virtual Stream& operator << (const bool v) = 0;

    //! @brief Streaming out a boolean value.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    virtual Stream& operator >> (bool& v) = 0;
};

//! @brief Streaming in data
/**
 * IStream is purely for streaming in data. Trying to stream out data with IStream will result in
 * crash. It is higher level code's responsibility to make sure it won't happen.
 */
class IStream : public Stream 
{
public:
    //! @brief Disable streaming in a float number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    Stream& operator << (const float v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStream!"); return *this; }

    //! @brief Disable streaming in a integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    Stream& operator << (const int v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStream!"); return *this; }

    //! @brief Disable streaming in an unsigned integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    Stream& operator << (const unsigned int v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStream!"); return *this; }

    //! @brief Disable streaming in a string. Attempting to do it will result in crash!
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    Stream& operator << (const std::string& v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStream!"); return *this; }

    //! @brief Disable streaming in a boolean value. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be saved.
    //! @return     Reference of the stream itself.
    Stream& operator << (const bool v) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStream!"); return *this; }
};

//! @brief Streaming out data
/**
 * OStream is purely for streaming in data. Trying to stream in data with OStream will result in
 * crash. It is higher level code's responsibility to make sure it won't happen.
 */
class OStream : public Stream
{
public:
    //! @brief Disable streaming out a float number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    Stream& operator >> (float& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }

    //! @brief Disable streaming out a integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    Stream& operator >> (int& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }

    //! @brief Disable streaming out an unsigned integer number. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    Stream& operator >> (unsigned int& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }

    //! @brief Disable streaming out a string. Attempting to do it will result in crash!
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    Stream& operator >> (std::string& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }

    //! @brief Disable streaming out a boolean value. Attempting to do it will result in crash!
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    Stream& operator >> (bool& v) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); return *this; }
};