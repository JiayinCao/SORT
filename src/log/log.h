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

#define slog( level , type , s ) sortLog( level , type , s , __FILE__ , __LINE__ )

enum class LOG_LEVEL{
    DEBUG = 0x01,
    INFO = 0x02,
    WARNING = 0x04,
    ERROR = 0x08,
    CRITICAL = 0x10
};

enum class LOG_TYPE{
    GENERAL = 0x01,
    SPATIAL_ACCELERATOR = 0x02,
    PERFORMANCE = 0x04,
    INTEGRATOR = 0x08,
    LIGHT = 0x10,
    MATERIAL = 0x20,
    IMAGE = 0x40,
};

class LogDispatcher{
public:
    // dispatch a log
    void dispatch( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line );
    
    // output result
    virtual void output( const string& s ) = 0;
    
private:
    // format log
    const string format( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ) const;
    
    // format log header
    const string formatHead( LOG_LEVEL level , LOG_TYPE type , const char* file , const int line ) const;
};

class FileLogDispatcher : public LogDispatcher {
public:
    FileLogDispatcher( const string& filename );
    ~FileLogDispatcher();
    
    void output( const string& s ) override;
private:
    // file to output
    ofstream file;
};

class StdOutLogDispatcher : public LogDispatcher {
public:
    void output( const string& s ) override ;
};

void sortLog( LOG_LEVEL level , LOG_TYPE type , const string& str , const char* file , const int line );
void addLogDispatcher( LogDispatcher* logdispatcher );
