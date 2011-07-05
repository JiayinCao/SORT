/*
 * filename :	path.cpp
 *
 * programmer :	Cao Jiayin
 */

// include header file
#include "path.h"

#if defined(SORT_IN_WINDOWS)
#include <windows.h>
#endif

// the resource path
static string g_ResourcePath = "";

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
	g_ResourcePath = str;
}

// get resource path
string GetResourcePath()
{
#if defined( SORT_IN_WINDOWS )
	string abspath = GetExecutableDir();
#elif defined( SORT_IN_LINUX )
	string abspath = "";
#endif
	if( g_ResourcePath.empty() )
		return abspath;

	// it's a relative path
	if( g_ResourcePath[0] == '.' )
		return abspath + g_ResourcePath;

	return g_ResourcePath;
}
