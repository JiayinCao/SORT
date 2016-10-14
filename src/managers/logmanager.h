/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_LOGMANAGER
#define	SORT_LOGMANAGER

// include the header file
#include "utility/singleton.h"
#include <fstream>

// a empty class for output a new line
class _ENDL{
public:
	_ENDL( bool flag ){ m_bCrash = flag; }
	
	//whether crash after the message
	bool m_bCrash;
};

/////////////////////////////////////////////////////////////
// definition of log manager
class	LogManager : public Singleton<LogManager>
{
// public method
public:
	// destructor
	~LogManager();
	
	// enable or disable log
	// para 'enable' : enable or disable the log system
	void SetLogEnabled( bool enable );
	
	// set the header of log
	void SetHeader( const string& str );

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
	LogManager( bool enable = true );
	// initialize the system
	void	_init();
	// release the system
	void	_release();

	// set friend class
	friend LogManager& operator<<( LogManager& , unsigned );
	friend LogManager& operator<<( LogManager& , int );
	friend LogManager& operator<<( LogManager& , float );
	friend LogManager& operator<<( LogManager& , const char* );
	friend LogManager& operator<<( LogManager& , const std::string& );
	friend LogManager& operator<<( LogManager& , _ENDL );

	friend class Singleton<LogManager>;
};

// declare some useful macro for convinience
#define	LOG_FILE_INFO	"(File:"<<__FILE__<<"  Line:"<<__LINE__<<") "
#define LOG				LogManager::GetSingleton()
#define LOG_WARNING		LOG<<LOG_FILE_INFO<<"WARNING: "
#define	LOG_ERROR		LOG<<LOG_FILE_INFO<<"ERROR: "
#define	LOG_HEADER(str)	LogManager::GetSingleton().SetHeader( str )

#define ENDL _ENDL(false)
#define	CRASH _ENDL(true)

// the log manager stream operator
LogManager& operator<<( LogManager& , unsigned );
LogManager& operator<<( LogManager& , int );
LogManager& operator<<( LogManager& , const char* );
LogManager& operator<<( LogManager& , float );
LogManager& operator<<( LogManager& , const std::string& );
LogManager& operator<<( LogManager& , _ENDL );

#endif
