/*
 * filename :	normal.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_NORMAL
#define	SORT_NORMAL

// include the header file
#include "vector.h"

////////////////////////////////////////////////////////////////////////
// defination of normal
class Normal : public Vector
{
// public method:
public:
	// default constructor
	Normal();
	// constructor from three float
	// para '_x' :	x component
	// para '_y' :	y component
	// para '_z' :	z component
	Normal( float _x , float _y , float _z );
	// copy constructor
	// para 'n'  :	the normal to be copied
	Normal( const Normal& n );
	// destructor
	virtual ~Normal();
};

#endif
