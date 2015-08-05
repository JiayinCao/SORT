/*
   FileName:      path.cpp

   Created Time:  2011-08-04 12:43:03

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "path.h"
#include "system.h"

#if defined(SORT_IN_WINDOWS)
#include <windows.h>
#elif defined(SORT_IN_MAC)
#include <libproc.h>
#include <unistd.h>
#endif


extern System g_System;

// get current directory
string GetExecutableDir()
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
	int len = strlen( buf );
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
    if ( ret <= 0 ) {
		LOG_ERROR<<"Can't get current directory."<<ENDL;
        return "";
    }
#endif

	return string(buf);
}

// get full path
string GetFullPath( const string& str )
{
	return GetResourcePath() + str;
}

// set resource path
void SetResourcePath( const string& str )
{
	g_System.SetResourcePath( str );
}

// get resource path
string GetResourcePath()
{
#if defined( SORT_IN_WINDOWS )
	string abspath = GetExecutableDir();
#elif defined( SORT_IN_LINUX ) || defined( SORT_IN_MAC )
	string abspath = "";
#endif
	string res_path = g_System.GetResourcePath();
	if( res_path.empty() )
		return abspath;

	// it's a relative path
	if( res_path[0] == '.' )
		return abspath + res_path;

	return res_path;
}
