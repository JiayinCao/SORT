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

#include "log.h"
#include <vector>
#include <memory>
#include <time.h>

static vector<unique_ptr<LogDispatcher>> logDispatcher;
static bool logLevel = true;
static bool logType = true;
static bool logTime = true;
static bool logLineInfo = false;
static LOG_LEVEL logDefaultLevel = LOG_LEVEL::LOG_DEBUG;     // By default, debug information is avoided.

void addLogDispatcher( LogDispatcher* logdispatcher ){
    logDispatcher.push_back( unique_ptr<LogDispatcher>(logdispatcher) );
}

void sortLog( LOG_LEVEL level , LOG_TYPE type , const string& str , const char* file , const int line ){
    if( level < logDefaultLevel )
        return;
    for( const auto& it : logDispatcher )
        it->dispatch( level , type , str.c_str() , file , line );
}

void LogDispatcher::dispatch( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ){
    output(format( level , type , str , file , line ));
}

const string logTimeString(){
    if( !logTime )
        return "";
    
    auto tostr = []( const int t , const int size = 2 ){
        string s = to_string(t);
        return string( size - s.size(), '0' ) + s;
    };
    
    time_t t = time(0);
    struct tm * now = localtime( &t );
    return "[" + tostr(now->tm_year + 1900 , 4) + '-' + tostr(now->tm_mon + 1) + '-' + tostr( now->tm_mday) + ' ' + tostr(now->tm_hour) + ':' + tostr(now->tm_min) + ':' + tostr( now->tm_sec) + "]";
}

const string levelToString( LOG_LEVEL level ){
    return !logLevel ? "" :
    ( LOG_LEVEL::LOG_DEBUG == level ) ? "[Debug]" :
    ( LOG_LEVEL::LOG_INFO == level ) ? "[Info]" :
    ( LOG_LEVEL::LOG_WARNING == level ) ? "[Warning]" :
    ( LOG_LEVEL::LOG_ERROR == level ) ? "[Error]" :
    ( LOG_LEVEL::LOG_CRITICAL == level ) ? "[Critical]" : "";
}

const string typeToString( LOG_TYPE type ){
    return !logType ? "" :
    ( LOG_TYPE::LOG_GENERAL == type ) ? "[General]" :
    ( LOG_TYPE::LOG_SPATIAL_ACCELERATOR == type ) ? "[Spatial Accelerator]":
    ( LOG_TYPE::LOG_PERFORMANCE == type ) ? "[Performance]" :
    ( LOG_TYPE::LOG_INTEGRATOR == type ) ? "[Integrator]" :
    ( LOG_TYPE::LOG_LIGHT == type ) ? "[Light]" :
    ( LOG_TYPE::LOG_MATERIAL == type ) ? "[Material]" :
    ( LOG_TYPE::LOG_IMAGE == type ) ? "[Image]" :
    ( LOG_TYPE::LOG_SAMPLING == type ) ? "[Sampling]" : "";
}

const string lineInfoString( const char* file , int line ){
    if( !logLineInfo )
        return "";
    return "[File:" + string(file) + "  Line:" + to_string(line) + "]";
}

// format log header
const string LogDispatcher::formatHead( LOG_LEVEL level , LOG_TYPE type , const char* file , const int line ) const{
    return logTimeString() + levelToString(level) + typeToString( type ) + lineInfoString( file , line ) + "\t";
}

const string LogDispatcher::format( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ) const{
    return formatHead( level , type , file , line ) + string(str);
}

void StdOutLogDispatcher::output( const string& s ){
    cout<<s<<endl;
}

FileLogDispatcher::FileLogDispatcher( const string& filename ){
    file.open( filename.c_str() );
}
FileLogDispatcher::~FileLogDispatcher(){
    file.close();
}

void FileLogDispatcher::output( const string& s ){
    if( !file.is_open() )
        return;
    file<<s<<endl;
}
