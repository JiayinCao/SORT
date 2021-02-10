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

#include "core/define.h"
#include <fstream>
#include <memory>

#define slog( level , type , ... ) \
[&]() \
{ \
    const std::size_t size = snprintf(nullptr, 0, __VA_ARGS__) + 1; \
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(size); \
    snprintf(buf.get(), size, __VA_ARGS__); \
    sortLog( LOG_LEVEL::LOG_##level , LOG_TYPE::LOG_##type , buf.get() , __FILE__ , __LINE__ );\
}()

enum class LOG_LEVEL {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL,
};

enum class LOG_TYPE{
    LOG_GENERAL,
    LOG_SPATIAL_ACCELERATOR,
    LOG_PERFORMANCE,
    LOG_INTEGRATOR,
    LOG_LIGHT,
    LOG_MATERIAL,
    LOG_VOLUME,
    LOG_IMAGE,
    LOG_SAMPLING,
    LOG_CAMERA,
    LOG_SHAPE,
    LOG_STREAM,
    LOG_RESOURCE,
    LOG_TASK,
    LOG_MEMORY,
    LOG_SOCKET,
};

//! @brief  LogDispatcher is an interface for dispatching log messages to different places.
class LogDispatcher{
public:
    //! @brief  Empty destructor.
    virtual ~LogDispatcher() {}

    //! @brief  Dispatch the log message. This guarantees the log is thread safe.
    //!
    //! @param  level       Level of the message.
    //! @param  type        Type of the message.
    //! @param  str         The message to be logged.
    //! @param  file        The name of the file where the logging happens.
    //! @param  line        The number of the line in the file where the logging happens.
    void Dispatch( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line );

private:
    //! @brief  Output the log message.
    //!
    //! @param  level       Level of the log.
    //! @param  header      Header of the message.
    //! @oaram  info        The message to be output.
    virtual void output( const LOG_LEVEL level , const std::string& header , const std::string& info ) = 0;

    //! @brief  Format the message.
    //!
    //! @param  level       Level of the message.
    //! @param  type        Type of the message.
    //! @param  str         The message to be logged.
    //! @param  file        The name of the file where the logging happens.
    //! @param  line        The number of the line in the file where the logging happens.
    const std::string format( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ) const;

    //! @brief  Format the message header.
    //!
    //! @param  level       Level of the message.
    //! @param  type        Type of the message.
    //! @param  file        The name of the file where the logging happens.
    //! @param  line        The number of the line in the file where the logging happens.
    const std::string formatHead( LOG_LEVEL level , LOG_TYPE type , const char* file , const int line ) const;
};

//! @brief  FileLogDispatcher dispatch logs to a file to be viewed afterward.
class FileLogDispatcher : public LogDispatcher {
public:
    //! @brief  Constructor automatically opens the file.
    //!
    //! @param  filename    Name of the log file.
    FileLogDispatcher( const std::string& filename );

    //! @brief  Destructor closes the file.
    ~FileLogDispatcher();

private:
    std::ofstream   m_file;     /**< File to be logged. */

    //! @brief  Output the log message to the file.
    //!
    //! @param  level       Level of the log.
    //! @param  header      Header of the message.
    //! @oaram  info        The message to be output.
    void output( const LOG_LEVEL level , const std::string& header , const std::string& info ) override ;
};

//! @brief   StdOutLogDispatcher outputs the message to std output.
class StdOutLogDispatcher : public LogDispatcher {
    //! @brief  Output the log message to the std-out.
    //!
    //! @param  level       Level of the log.
    //! @param  header      Header of the message.
    //! @oaram  info        The message to be output.
    void output( const LOG_LEVEL level , const std::string& header , const std::string& info ) override ;
};

//! @brief  Format the message.
//!
//! @param  level       Level of the message.
//! @param  type        Type of the message.
//! @param  str         The message to be logged.
//! @param  file        The name of the file where the logging happens.
//! @param  line        The number of the line in the file where the logging happens.
void sortLog( LOG_LEVEL level , LOG_TYPE type , const std::string& str , const char* file , const int line );

//! @brief  Add a dispatcher to the log system.
//!
//! @param  logDispatcher   Dispatcher to add in the log system.
void addLogDispatcher( std::unique_ptr<LogDispatcher> logDispatcher );

//! @brief  Utility function to get current time.
std::string logTimeString();

//! @brief  Utility function go get current time
std::string logTimeStringStripped();