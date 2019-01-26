/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <string>
#include <mutex>
#include "core/path.h"

static std::vector<std::unique_ptr<LogDispatcher>> g_logDispatcher;
static bool g_logLevel = true;
static bool g_logType = true;
static bool g_logTime = true;
static bool g_logLineInfo = false;
static LOG_LEVEL logDefaultLevel = LOG_LEVEL::LOG_DEBUG;     // By default, debug information is avoided.

void addLogDispatcher( std::unique_ptr<LogDispatcher> logDispatcher ){
    g_logDispatcher.push_back( std::move(logDispatcher) );
}

void sortLog( LOG_LEVEL level , LOG_TYPE type , const std::string& str , const char* file , const int line ){
    if( level < logDefaultLevel )
        return;
    for( const auto& it : g_logDispatcher )
        it->Dispatch( level , type , str.c_str() , file , line );
}

void LogDispatcher::Dispatch( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ){
    const auto header = formatHead( level , type , file , line );
    const auto info = std::string( str );
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    output(level,header,info);
}

const std::string logTimeString(){
    if( !g_logTime )
        return "";
    
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char s[128] = { 0 };

#ifdef SORT_IN_WINDOWS
    struct tm t;
    ::localtime_s(&t, &now);
    std::strftime(s, 128, "[%Y-%m-%d %H:%M:%S]", &t);
#else
    std::strftime(s, 128, "[%Y-%m-%d %H:%M:%S]", std::localtime(&now) );
#endif
    return std::string(s);
}

const std::string levelToString( LOG_LEVEL level ){
    return !g_logLevel ? "" :
    ( LOG_LEVEL::LOG_DEBUG == level ) ? "[Debug]" :
    ( LOG_LEVEL::LOG_INFO == level ) ? "[Info]" :
    ( LOG_LEVEL::LOG_WARNING == level ) ? "[Warning]" :
    ( LOG_LEVEL::LOG_ERROR == level ) ? "[Error]" :
    ( LOG_LEVEL::LOG_CRITICAL == level ) ? "[Critical]" : "";
}

const std::string levelToHeaderColorCode( LOG_LEVEL level ){
    return !g_logLevel ? "\033[39m" :
    ( LOG_LEVEL::LOG_DEBUG == level ) ? "\033[33m" :
    ( LOG_LEVEL::LOG_INFO == level ) ? "\033[32m" :
    ( LOG_LEVEL::LOG_WARNING == level ) ? "\033[31m" :
    ( LOG_LEVEL::LOG_ERROR == level ) ? "\033[35m" :
    ( LOG_LEVEL::LOG_CRITICAL == level ) ? "\033[35m" : "\033[39m";
}

const std::string typeToString( LOG_TYPE type ){
    return !g_logType ? "" :
    ( LOG_TYPE::LOG_GENERAL == type ) ? "[General]" :
    ( LOG_TYPE::LOG_SPATIAL_ACCELERATOR == type ) ? "[Spatial Accelerator]":
    ( LOG_TYPE::LOG_PERFORMANCE == type ) ? "[Performance]" :
    ( LOG_TYPE::LOG_INTEGRATOR == type ) ? "[Integrator]" :
    ( LOG_TYPE::LOG_LIGHT == type ) ? "[Light]" :
    ( LOG_TYPE::LOG_MATERIAL == type ) ? "[Material]" :
    ( LOG_TYPE::LOG_IMAGE == type ) ? "[Image]" :
    ( LOG_TYPE::LOG_SAMPLING == type ) ? "[Sampling]" :
    ( LOG_TYPE::LOG_CAMERA == type ) ? "[Camera]" : 
    ( LOG_TYPE::LOG_SHAPE == type ) ? "[Shape]" :
    ( LOG_TYPE::LOG_STREAM == type ) ? "[Stream]" :
    ( LOG_TYPE::LOG_RESOURCE == type ) ? "[Resource]" :
    ( LOG_TYPE::LOG_TASK == type ) ? "[Task]" : "[Unknown]";
}

const std::string lineInfoString( const char* file , int line , LOG_LEVEL level ){
    if( !g_logLineInfo && level != LOG_LEVEL::LOG_CRITICAL )
        return "";
    return "[File:" + std::string(file) + "  Line:" + std::to_string(line) + "]";
}

const std::string LogDispatcher::formatHead( LOG_LEVEL level , LOG_TYPE type , const char* file , const int line ) const{
    return logTimeString() + levelToString(level) + typeToString( type ) + lineInfoString( file , line , level ) + "\t";
}

const std::string LogDispatcher::format( LOG_LEVEL level , LOG_TYPE type , const char* str , const char* file , const int line ) const{
    return formatHead( level , type , file , line ) + std::string(str);
}

void StdOutLogDispatcher::output( const LOG_LEVEL level , const std::string& header , const std::string& info ){
    const auto color_code = levelToHeaderColorCode( level );
    static const auto default_color_code = 39;
    std::cout<<color_code<<header<<"\033[39m"<<" "<<info<<std::endl;
}

FileLogDispatcher::FileLogDispatcher( const std::string& filename ){
    m_file.open( GetFilePathInExeFolder(filename) );
}

FileLogDispatcher::~FileLogDispatcher(){
    m_file.close();
}

void FileLogDispatcher::output( const LOG_LEVEL level , const std::string& header , const std::string& info ){
    if( !m_file.is_open() )
        return;
    m_file<<header<<" "<<info<<std::endl;
}
