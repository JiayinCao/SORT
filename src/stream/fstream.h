/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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
#include "stream/mstream.h"

//! @brief Streaming from file.
/**
 * IFileStream only works for streaming data from a file. Any attempt to write data
 * to a file will result in immediate crash.
 */
class IFileStream : public IStreamBase{
public:
    //! @brief Constructing from a file name.
    //!
    //! @param filename     Name of the file to be streamed.
    IFileStream(const std::string& filename) {
        m_file.open(filename, std::ios::in | std::ios::binary );
        if (!m_file.is_open())
            slog(WARNING, STREAM, "File %s can't be loaded.", filename.c_str());
    }

    //! @brief Destructor will close the file.
    ~IFileStream() {
        Close();
    }

    //! @brief Open a new stream file.
    //!
    //! @param filename     Name of the file to be streamed to.
    //! @param force        Forcing to open the stream file if there is already a file open. Default value is 'true'
    //! @return             Whether the new streamed file is opened. It could failed due to several reasons, like no
    //!                     access to the file, no right to open the file or there is already a file opened, while
    //!                     @param force is false.
    bool    Open( const std::string& filename , bool force = true ){
        if( m_file.is_open() ){
            if( force )
                m_file.close();
            else
                return false;
        }

        m_file.open( filename.c_str() );
        return m_file.is_open();
    }

    //! @brief Close the currently opened stream file.
    //!
    //! @return             It returns true if there is a currently opened file, otherwise false will be returned.
    bool    Close(){
        if( !m_file.is_open() )
            return false;
        m_file.close();
        return true;
    }

    //! @brief Streaming in a float number to file.
    //!
    //! @param v            Value to be loaded.
    //! @return             Reference of the stream itself.
    StreamBase& operator >> (float& v) override {
        m_file.read(reinterpret_cast<char*>(&v), sizeof(float));
        return *this;
    }

    //! @brief Streaming in an integer number to file.
    //!
    //! @param v            Value to be loaded.
    //! @return             Reference of the stream itself.
    StreamBase& operator >> (int& v) override {
        m_file.read(reinterpret_cast<char*>(&v), sizeof(int));
        return *this;
    }

    //! @brief Streaming out an 8 bit integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator >> (char& v) override {
        m_file.read(reinterpret_cast<char*>(&v), sizeof(int));
        return *this;
    }

    //! @brief Streaming in an unsigned integer number to file.
    //!
    //! @param v            Value to be loaded.
    //! @return             Reference of the stream itself.
    StreamBase& operator >> (unsigned int& v) override {
        m_file.read(reinterpret_cast<char*>(&v), sizeof(unsigned int));
        return *this;
    }

    //! @brief Streaming in a string to file.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v            Value to be loaded.
    //! @return             Reference of the stream itself.
    StreamBase& operator >> (std::string& v) override {
        v = "";
        char c;
        do{
            m_file.read(reinterpret_cast<char*>(&c), sizeof(char));
            if( c == 0 )
                break;
            v += c;
        }while(true);
        return *this;
    }

    //! @brief Streaming in a boolean value to file.
    //!
    //! @param v            Value to be loaded.
    //! @return             Reference of the stream itself.
    StreamBase& operator >> (bool& v) override {
        m_file.read(reinterpret_cast<char*>(&v), sizeof(bool));
        return *this;
    }

    //! @brief Loading data from stream directly.
    //!
    //! @param  data    Data to be filled.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Load( char* data , int size ) override {
        m_file.read(data, size);
        return *this;
    }

private:
    std::ifstream m_file;       /**< File to be streamed from. */
};

//! @brief Streaming to file.
/**
 * OFileStream only works for streaming data to a file. Any attempt to read data
 * from a file will result in immediate crash.
 */
class OFileStream : public OStreamBase{
public:
    //! @brief Constructing from a file name.
    //!
    //! @param filename     Name of the file to be streamed.
    OFileStream(const std::string& filename) {
        m_file.open(filename, std::ios::out | std::ios::binary );
        if (!m_file.is_open())
            slog(WARNING, STREAM, "File %s can't be loaded.", filename.c_str());
    }

    //! @brief Destructor will close the file.
    ~OFileStream() {
        Close();
    }

    //! @brief Open a new stream file.
    //!
    //! @param filename     Name of the file to be streamed to.
    //! @param force        Forcing to open the stream file if there is already a file open. Default value is 'true'
    //! @return             Whether the new streamed file is opened. It could failed due to several reasons, like no
    //!                     access to the file, no right to open the file or there is already a file opened, while
    //!                     @param force is false.
    SORT_FORCEINLINE bool    Open( const std::string& filename , bool force = true ){
        if( m_file.is_open() ){
            if( force )
                m_file.close();
            else
                return false;
        }

        m_file.open( filename.c_str() );
        return m_file.is_open();
    }

    //! @brief Close the currently opened stream file.
    //!
    //! @return             It returns true if there is a currently opened file, otherwise false will be returned.
    SORT_FORCEINLINE bool    Close(){
        if( !m_file.is_open() )
            return false;
        m_file.close();
        return true;
    }

    //! @brief Streaming out a float number from file.
    //!
    //! @param v            Value to be saved.
    //! @return             Reference of the stream itself.
    StreamBase& operator << (const float v) override {
        m_file.write(reinterpret_cast<const char*>(&v), sizeof(float));
        return *this;
    }

    //! @brief Streaming out an integer number from file.
    //!
    //! @param v            Value to be saved.
    //! @return             Reference of the stream itself.
    StreamBase& operator << (const int v) override {
        m_file.write(reinterpret_cast<const char*>(&v), sizeof(int));
        return *this;
    }

    //! @brief Streaming out an 8 bit integer number.
    //!
    //! @param v    Value to be loaded.
    //! @return     Reference of the stream itself.
    StreamBase& operator << (char v) override {
        m_file.write(reinterpret_cast<const char*>(&v), sizeof(int));
        return *this;
    }

    //! @brief Streaming out an unsigned integer number from file.
    //!
    //! @param v            Value to be saved.
    //! @return             Reference of the stream itself.
    StreamBase& operator << (const unsigned int v) override {
        m_file.write(reinterpret_cast<const char*>(&v), sizeof(unsigned int));
        return *this;
    }

    //! @brief Streaming out a string from file.
    //!
    //! Unlike stand stream, space doesn't count to separate strings. For example, streaming "hello world" in will
    //! result in one single string instead of two.
    //!
    //! @param v            Value to be saved.
    //! @return             Reference of the stream itself.
    StreamBase& operator << (const std::string& v) override {
        if( !v.empty() )
            m_file.write(v.c_str(), v.size());
        static const char end = 0;
        m_file.write(&end, sizeof(end));
        return *this;
    }

    //! @brief Streaming out a boolean value from file.
    //!
    //! @param v            Value to be saved.
    //! @return             Reference of the stream itself.
    StreamBase& operator << (const bool v) override {
        m_file.write(reinterpret_cast<const char*>(&v), sizeof(bool));
        return *this;
    }

    //! @brief Writing data to stream.
    //!
    //! @param  data    Data to be written.
    //! @param  size    Size of the data to be filled in bytes.
    StreamBase& Write( char* data , int size ) override {
        m_file.write(data, size);
        return *this;
    }

    //! @brief Flush current written result.
    void Flush() override{
        m_file.flush();
    }

    //! @brief Get current position. Ideally, IStreamBase might need to have this interface too. But it is not currently needed.
    unsigned    GetPos() override {
        return m_file.tellp();
    }

    //! @brief Seek to a specific position
    void        Seek(unsigned int p) override {
        m_file.seekp(p);
    }

private:
    std::ofstream m_file;       /**< File to be streamed to. */
};