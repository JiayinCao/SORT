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

// include header file
#include "path.h"
#include "system.h"

#if defined(SORT_IN_WINDOWS)
#include <windows.h>
#elif defined(SORT_IN_MAC)
#include <libproc.h>
#include <unistd.h>
#elif defined(SORT_IN_LINUX)
#include <unistd.h>
#endif


extern System g_System;

// get current directory
std::string GetExecutableDir()
{
	const int maxLen = 2048;
	char buf[ maxLen ];

#if defined(SORT_IN_LINUX)
	int c = readlink( "/proc/self/exe", buf, maxLen - 1 );
	buf[c] = '/';
	buf[c+1] = 0;
#elif defined(SORT_IN_WINDOWS)
	// get the current module filename
	GetModuleFileNameA( NULL , buf , maxLen );

	// remove the file name
	int len = (int)strlen( buf );
	for( int i = len - 1 ; i > 0 ; i-- )
	{
		if( buf[i] == '\\' )
		{
			buf[i+1] = 0;
			break;
		}
	}
#elif defined(SORT_IN_MAC)
	pid_t pid = getpid();
    int ret = proc_pidpath (pid, buf, sizeof(buf));
    sAssertMsg( ret > 0 , GENERAL , "Can't get current directory." );

	int i = strlen(buf);
	while( i > 0 && buf[i] != '/' )
		buf[i--] = 0;
#endif

	return std::string(buf);
}

// get full path
std::string GetFullPath( const std::string& str )
{
	return GetResourcePath() + str;
}

// set resource path
void SetResourcePath( const std::string& str )
{
	g_System.SetResourcePath( str );
}

// get resource path
std::string GetResourcePath()
{
	return g_System.GetResourcePath();
}
