/*
 * filename :	logmanager.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "logmanager.h"
#include "utility/path.h"

// instance the singleton with logmanager class
DEFINE_SINGLETON(LogManager);

// create a log manager
void	LogManager::CreateLogManager( bool enable )
{
	LogManager* m = new LogManager( enable );

	if( enable == false )
		m->m_fileLog<<"Log disabled."<<endl;
}

// private constructor
LogManager::LogManager( bool enable ):
	m_sLogFileName( GetFullPath("log_sort.txt") )
{
	_init();

	m_bLogEnable = enable;
}

// destructor
LogManager::~LogManager()
{
	_release();
}

// initialize the log manager
void LogManager::_init()
{
	// try to open the log file
	m_fileLog.open( m_sLogFileName.c_str() );

	// output some help information
	m_fileLog<<"-------------------------------------------------------------------------------------------------"<<endl;
	m_fileLog<<"                                This is the log file for SORT."<<endl;
	m_fileLog<<"-------------------------------------------------------------------------------------------------"<<endl;
}

// release the log manager
void LogManager::_release()
{
	m_fileLog.close();
}

// enable or disable the log
void LogManager::SetLogEnabled( bool enable )
{
	m_bLogEnable = enable;

	if( enable == false )
		m_fileLog<<"Log disabled."<<endl;
}

// output message to the logger
LogManager& operator<<( LogManager& manager , unsigned data )
{
	if( manager.m_bLogEnable == false )
		return manager;

	manager.m_fileLog<<data;

	return manager;
}

LogManager& operator<<( LogManager& manager , int data )
{
	if( manager.m_bLogEnable == false )
		return manager;

	manager.m_fileLog<<data;

	return manager;
}
LogManager& operator<<( LogManager& manager , float data )
{
	if( manager.m_bLogEnable == false )
		return manager;

	manager.m_fileLog<<data;
	
	return manager;
}
LogManager& operator<<( LogManager& manager , const char* data )
{
	if( manager.m_bLogEnable == false )
		return manager;

	manager.m_fileLog<<data;

	return manager;
}
LogManager& operator<<( LogManager& manager , _ENDL data )
{
	if( manager.m_bLogEnable == false )
		return manager;

	manager.m_fileLog<<endl;

	//crash
	if( data.m_bCrash )
		abort();

	return manager;
}
LogManager& operator<<( LogManager& manager , const std::string& str )
{
	if( manager.m_bLogEnable == false )
		return manager;

	manager.m_fileLog<<str.c_str();

	return manager;
}
