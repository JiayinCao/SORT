/*
 * filename :	path.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_PATH
#define	SORT_PATH

// get current directory
string GetExecutableDir()
{
	const int maxLen = 2048;
	char buf[ maxLen ];

#if defined(SORT_IN_LINUX)
	int c = readlink( "/proc/self/exe", buf, maxLen - 1 );
#elif defined(SORT_IN_WINDOWS)

#endif

	return string(buf);
}

#endif
