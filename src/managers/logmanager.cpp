/*
 * filename :	logmanager.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "logmanager.h"

// instance the singleton with logmanager class
DEFINE_SINGLETON(LogManager);

// create a log manager
void	LogManager::CreateLogManager( bool enable )
{
	new LogManager( enable );
}

// private constructor
LogManager::LogManager( bool enable ):
	m_sLogFileName( "log_sort.txt" )
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

// add new log
void LogManager::Log( const std::string& str , bool enter )
{
	if( m_bLogEnable )
	{
		m_fileLog<<str;

		if( enter )
			m_fileLog<<endl;
	}
}

// enable or disable the log
void LogManager::SetLogEnabled( bool enable )
{
	m_bLogEnable = enable;
}
