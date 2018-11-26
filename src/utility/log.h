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

#include "utility/singleton.h"
#include <fstream>

#define slog( level , type , s ) sortLog( LOG_LEVEL::LOG_##level , LOG_TYPE::LOG_##type , s , __FILE__ , __LINE__ )

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
    LOG_IMAGE,
    LOG_SAMPLING,
    LOG_CAMERA,
    LOG_SHAPE,
    LOG_STREAM,
};

class LogDispatcher{
public:
    // virtual destructor
    virtual ~LogDispatcher() {}

    // dispatch a log
    void dispatch( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line );
    
    // output result
    virtual void output( const std::string& s ) = 0;
    
private:
    // format log
    const std::string format( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ) const;
    
    // format log header
    const std::string formatHead( LOG_LEVEL level , LOG_TYPE type , const char* file , const int line ) const;
};

class FileLogDispatcher : public LogDispatcher {
public:
    FileLogDispatcher( const std::string& filename );
    ~FileLogDispatcher();
    
    void output( const std::string& s ) override;
private:
    // file to output
    std::ofstream file;
};

class StdOutLogDispatcher : public LogDispatcher {
public:
    void output( const std::string& s ) override ;
};

void sortLog( LOG_LEVEL level , LOG_TYPE type , const std::string& str , const char* file , const int line );
void addLogDispatcher( LogDispatcher* logdispatcher );
