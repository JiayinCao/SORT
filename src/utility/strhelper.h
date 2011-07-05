/*
 * filename :	strhelper.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_STRHELPER
#define	SORT_STRHELPER

#include "sort.h"
#include "enum.h"
#include "geometry/transform.h"

// pre-decleration
struct VertexIndex;

////////////////////////////////////////////////////////////////////////
// there are some global functions for parsing string

// convert string to vertex index
// para 'str' : the string to parse
// result     : a vertex index
VertexIndex	VertexIndexFromStr( const string& str );

// get the type of file according to the file extension
// para 'str' : string to parse
// result     : a cooresponding mesh type
MESH_TYPE	MeshTypeFromStr( const string& str );

// get the type of image file from file extension
// para 'str' : string to parse
// result     : a cooresponding mesh type
TEX_TYPE	TexTypeFromStr( const string& str );

class ToLower{
public:
     char operator()(char val){   
        return tolower( val );
    }
};

// transform from string
Transform TransformFromStr( const string& str );

// get the next token
string NextToken( string& str , char t );

#endif
