/*
 * filename :	primitive.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_PRIMITIVE
#define	SORT_PRIMITIVE

// pre-decleration
class Ray;

//////////////////////////////////////////////////////////////////
//	defination of primitive
class	Primitive
{
// public method
public:
	// default constructor
	Primitive(){}
	// destructor
	~Primitive(){}

	// get the intersection between a ray and a primitive
	virtual float	GetIntersect( const Ray& r ) const = 0;

// private field
private:

};

#endif
