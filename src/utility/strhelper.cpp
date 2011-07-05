/*
 * filename :	strhelper.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "strhelper.h"
#include "managers/meshmanager.h"
#include <algorithm>
#include "geometry/transform.h"

// convert string to vertex index
VertexIndex	VertexIndexFromStr( const string& str )
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
	std::transform(substr.begin(),substr.end(),substr.begin(),ToLower());

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
	transform(substr.begin(),substr.end(),substr.begin(),ToLower());

	if( strcmp( substr.c_str() , "bmp" ) == 0 )
		return TT_BMP;

	// log a warning
	LOG_WARNING<<"Image type of \""<<substr<<"\" is not supported."<<ENDL;

	return TT_END;
}

// transformation from string
Transform TransformFromStr( const string& s )
{
	// if there is no such an value , just return identity matrix and log a warning
	if( s.empty() )
	{
		LOG_WARNING<<"No value set in the transformation"<<ENDL;
		return Transform();
	}

	string str = s;

	// get the first character
	string t = NextToken( str , ' ' );
	if( t[0] == 't' )
	{
		t = NextToken( str , ' ' );
		float x = (float)atof( t.c_str() );
		t = NextToken( str , ' ' );
		float y = (float)atof( t.c_str() );
		t = NextToken( str , ' ' );
		float z = (float)atof( t.c_str() );
		return Translate( x , y , z );	
	}else if( t[0] == 'r' )
	{
		t = NextToken( str , ' ' );
		int axis = atoi( t.c_str() );
		t = NextToken( str , ' ' );
		float angle = (float)atof( t.c_str() );
		switch( axis )
		{
			case 0:
				return RotateX( angle );
			case 1:
				return RotateY( angle );
			case 2:
				return RotateZ( angle );
		}
	}else if( t[0] == 's' )
	{
		t = NextToken( str , ' ' );
		float s0 = (float)atof( t.c_str() );
		float s1 = s0;
		float s2 = s0;
		
		t = NextToken( str , ' ' );
		if( str.empty() == false )
		{
			s1 = (float)atof( t.c_str() );
			t = NextToken( str , ' ' );
			s2 = (float)atof( t.c_str() );
		}

		return Scale( s0 , s1 , s2 );
	}

	return Transform();
}

// get the next token
string NextToken( string& str , char t )
{
	// get the next t index
	unsigned id = str.find_first_of( t );
	while( id == 0 )
	{
		// get to the next one
		str = str.substr( 1 , string::npos );
		id = str.find_first_of( t );
	}
	string res = str.substr( 0 , id );

	// if there is no such a character , set it none
	if( id == string::npos )
		str = "";
	else
		str = str.substr( id + 1 , string::npos );

	return res;
}
