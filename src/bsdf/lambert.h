/*
   FileName:      lambert.h

   Created Time:  2011-08-04 12:52:33

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_LAMBERT
#define	SORT_LAMBERT

// include header file
#include "bxdf.h"

////////////////////////////////////////////////////////////
// definition of lambert brdf
class Lambert : public Bxdf
{
// public method
public:
	// default constructor
	Lambert(){ m_type=BXDF_DIFFUSE; }
	// constructor
	// para 's' : reflect density
	Lambert( const Spectrum& s ):R(s){m_type=BXDF_DIFFUSE;}
	// destructor
	~Lambert(){}

	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const;

	// set color
	void SetColor( const Spectrum& color ) { R = color; }

// private field
private:
	// the total reflectance
	Spectrum R;
};

#endif
