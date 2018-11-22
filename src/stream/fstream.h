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
#include "utility/strhelper.h"

//! @brief Streaming from file.
/**
 * IFileStream only works for stream data from a file. Any attempt to write data
 * to a file will result in immediate crash.
 */
class IFileStream : public IStream
{
public:
    //! @brief Constructing from a file name.
    IFileStream(const std::string& filename) {
        m_file.open(filename, std::ios::in | std::ios::binary );
        if (!m_file.is_open())
            slog(WARNING, STREAM, stringFormat("File %s can't be loaded.", filename.c_str()));
    }

    //! @brief Destructor will close the file.
    ~IFileStream() {
        if(m_file.is_open())
            m_file.close();
    }

    //! @brief Streaming in a float number to file
    //!
    //! @param t    Value to be serialized
    void operator >> (float& t) override {
        m_file.read(reinterpret_cast<char*>(&t), sizeof(float));
    }

private:
    std::ifstream m_file;
};

//! @brief Streaming to file.
/**
 * OFileStream only works for stream data to a file. Any attempt to read data
 * from a file will result in immediate crash.
 */
class OFileStream : public OStream
{
public:
    //! @brief Constructing from a file name.
    OFileStream(const std::string& filename) {
        m_file.open(filename, std::ios::out | std::ios::binary );
        if (!m_file.is_open())
            slog(WARNING, STREAM, stringFormat("File %s can't be loaded.", filename.c_str()));
    }

    //! @brief Destructor will close the file.
    ~OFileStream() {
        if (m_file.is_open())
            m_file.close();
    }

    //! @brief Streaming out a float number from file
    //!
    //! @param t    Value to be saved
    void operator << (const float t) override {
        m_file.write(reinterpret_cast<const char*>(&t), sizeof(float));
    }

private:
    std::ofstream m_file;
};