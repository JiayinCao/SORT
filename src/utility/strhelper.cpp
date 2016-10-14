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

// include the header
#include "strhelper.h"
#include "managers/meshmanager.h"
#include <algorithm>
#include "math/transform.h"

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

	return MT_NONE;
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
	else if( strcmp( substr.c_str() , "exr" ) == 0 )
		return TT_EXR;
	else if( strcmp( substr.c_str() , "tga" ) == 0 )
		return TT_TGA;
	else if( strcmp( substr.c_str() , "jpg" ) == 0 )
		return TT_JPG;
    else if( strcmp( substr.c_str() , "png" ) == 0 )
        return TT_PNG;
	else if( strcmp( substr.c_str() , "hdr" ) == 0 )
		return TT_HDR;

	// log a warning
	LOG_WARNING<<"Image type of \""<<substr<<"\" is not supported."<<ENDL;

	return TT_NONE;
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
	}else if( t[0] == 'm' )
	{
		Matrix m;
		for( int i = 0 ; i < 16; ++i )
		{
			t = NextToken( str , ' ' );
			m.m[i] = (float)atof( t.c_str() );
		}
		return FromMatrix(m);
	}

	return Transform();
}

// spectrum from string
Spectrum SpectrumFromStr( const string& s )
{
	// if the string is empty , return black color
	if( s.empty() )
		return Spectrum();

	string str = s;
	string r = NextToken( str , ' ' );
	string g = NextToken( str , ' ' );
	string b = NextToken( str , ' ' );

	float fr = (float)atof( r.c_str() );
	float fg = (float)atof( g.c_str() );
	float fb = (float)atof( b.c_str() );

	return Spectrum( fr , fg , fb );
}

// point from string
Point PointFromStr( const string& s )
{
	// if the string is empty , return black color
	if( s.empty() )
		return Point();

	string str = s;
	string x = NextToken( str , ' ' );
	string y = NextToken( str , ' ' );
	string z = NextToken( str , ' ' );

	float fx = (float)atof( x.c_str() );
	float fy = (float)atof( y.c_str() );
	float fz = (float)atof( z.c_str() );

	return Point( fx , fy , fz );
}

// vector from string
Vector VectorFromStr( const string& s )
{
	if( s.empty() )
		return Vector();
	string str = s;
	string x = NextToken( str , ' ' );
	string y = NextToken( str , ' ' );
	string z = NextToken( str , ' ' );

	float fx = (float)atof( x.c_str() );
	float fy = (float)atof( y.c_str() );
	float fz = (float)atof( z.c_str() );

	return Vector( fx , fy , fz ); 
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
	if( id == (unsigned)string::npos )
		str = "";
	else
		str = str.substr( id + 1 , string::npos );

	return res;
}
