/*
 * filename :	strhelper.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "strhelper.h"
#include "managers/meshmanager.h"

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
