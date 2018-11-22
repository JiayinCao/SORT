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

    //! @brief Streaming in a float number
    //!
    //! @param t    Value to be saved
    virtual void operator << (const float t) = 0;

    //! @brief Streaming out a float number.
    //!
    //! @param t    Value to be loaded
    virtual void operator >> (float& t) = 0;
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
    void operator << (const float t) override final { sAssertMsg(false, STREAM, "Streaming in data by using OStream!"); }
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
    void operator >> (float& t) override final { sAssertMsg(false, STREAM, "Streaming out data by using IStream!"); }
};