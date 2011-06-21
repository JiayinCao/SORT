/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "system.h"
#include "geometry/transform.h"

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	Transform t;
	Vector* n = new Vector( true );
	Vector* v = new Vector();

	Vector n0 = t * (*n);
	Vector v0 = t * (*v);

	Vector* n1 = n;

	t * (*n1);

	delete n;
	delete v;

	return 0;
}
