/*
 * filename :	logmanager.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_LOGMANAGER
#define	SORT_LOGMANAGER

// include the header file
#include "../utility/singleton.h"
#include <fstream>

/////////////////////////////////////////////////////////////
// defination of log manager
class	LogManager : public Singleton<LogManager>
{
// public method
public:
	// create the logmanager
	static void CreateLogManager( bool enable = true );
	// destructor
	~LogManager();
	// record new log
	// para 'log' : log message
	// para 'enter' : wheter there is an enter appended
	void Log( const std::string& str  , bool enter = true );
	// enable or disable log
	// para 'enable' : enable or disable the log system
	void SetLogEnabled( bool enable );
	
// private method
private:
	// the logger file
	std::ofstream 	m_fileLog;
	// the file name of logger
	const std::string	m_sLogFileName;
	// enable log
	bool	m_bLogEnable;

// private method
	// private constructor to avoid second instance in the app
	LogManager() {}
	LogManager( bool enable );
	// initialize the system
	void	_init();
	// release the system
	void	_release();
};

// declare some useful macro for convinience
#define LOG(s)	LogManager::GetSingleton().Log(s)
#define	SLOG(s)	LogManager::GetSingleton().Log(s,false)

#endif
