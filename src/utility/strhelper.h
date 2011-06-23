/*
 * filename :	strhelper.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_STRHELPER
#define	SORT_STRHELPER

#include "sort.h"

struct VertexIndex;

////////////////////////////////////////////////////////////////////////
// there are some global functions for parsing string

// convert string to vertex index
// para 'str' : the string to parse
// result     : a vertex index
VertexIndex	Str2VertexIndex( const string& str );

#endif
