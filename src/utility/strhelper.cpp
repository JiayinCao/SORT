/*
 * filename :	strhelper.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "strhelper.h"
#include "managers/meshmanager.h"
#include <algorithm>

// convert string to vertex index
VertexIndex	Str2VertexIndex( const string& str )
{
	VertexIndex vi;
	string temp , rest;

	// get the position index
	int t0 = str.find_first_of( '/' );
	temp = str.substr( 0 , t0 );
	vi.posIndex = atoi( temp.c_str() ) - 1;
	if( t0 == string::npos )
		return vi;
	
	// get the texture coordinate index
	rest = str.substr( t0 + 1 , string::npos );
	t0 = rest.find_first_of( '/' );
	temp = rest.substr( 0 , t0 );
	vi.texIndex = temp.empty()?(-1):(atoi( temp.c_str() ) - 1);
	if( t0 == string::npos )
		return vi;

	// get the normal index
	rest = rest.substr( t0 + 1 , string::npos );
	vi.norIndex = rest.empty()?(-1):(atoi( rest.c_str() ) - 1 );

	return vi;
}

// get the type of file according to the file extension
MESH_TYPE	MeshTypeFromStr( const string& str )
{
	// get the file extension
	int index = str.find_last_of( "." );
	string substr = str.substr( index + 1 , str.length() - index );

	// transform the extension to lower case
	transform(substr.begin(),substr.end(),substr.begin(),tolower);

	if( strcmp( substr.c_str() , "obj" ) == 0 )
		return MT_OBJ;
	else if( strcmp( substr.c_str() , "ply" ) == 0 )
		return MT_PLY;
		
	// log a warning
	LOG_WARNING<<"Mesh type of \""<<substr<<"\" is not supported."<<ENDL;

	return MT_END;
}

// get the type of image file from file extension
TEX_TYPE	TexTypeFromStr( const string& str )
{
	// get the file extension
	int index = str.find_last_of( "." );
	string substr = str.substr( index + 1 , str.length() - index );

	// transform the extension to lower case
	transform(substr.begin(),substr.end(),substr.begin(),tolower);

	if( strcmp( substr.c_str() , "bmp" ) == 0 )
		return TT_BMP;

	// log a warning
	LOG_WARNING<<"Image type of \""<<substr<<"\" is not supported."<<ENDL;

	return TT_END;
}