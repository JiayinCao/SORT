/*
 * filename: sort.cpp
 *
 * programmer: Cao Jiayin
 */

// include the header file
#include <iostream>
#include "geometry/vector.h"
#include "geometry/point.h"
#include "geometry/normal.h"
#include "texture/texture.h"
#include "texture/texshower/bmpsaver.h"

// use standard namespace
using namespace std;

void output( const string& str , const Vector& v )
{
	cout<<"vector "<<str<<":"<<v.x<<" "<<v.y<<" "<<v.z<<endl;
}
void output( const string& str , const Point& p )
{
	cout<<"point "<<str<<":"<<p.x<<" "<<p.y<<" "<<p.z<<endl;
}
// the enter point for the program
int main( int argc , char** argv )
{
/*	Vector a;
	Vector b( 1 , 2 , 3 );
	Vector c(b);
	a = 4.0f * c + 0 * b;

	Point p( 1 , 0 , 3 );

	output( "a" , a + p );
	output( "b" , b );
	output( "c" , c );

	output( "a+b" , a+b );

	cout<<"a*b"<<" "<<Dot( a , b )<<endl;
	output( "axb" , Cross( a , b ) );

	cout<<"length(a)="<<a.Length()<<endl;
	
	output("unit a=",a.Normalize());

	Normal v( 1 , 2 , 4 );

	if( FaceForward( v , c ) )
		cout<<"win"<<endl;
	else
		cout<<"lose"<<endl;
*/
	Texture tex;
	TexShower* t = new BmpSaver();
	tex.SetTexShower( t );
	tex.Output( "hello.bmp" );

	Texture::SetTexShower( 0 );

  	return 0;
} 
