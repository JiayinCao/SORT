/*
   FileName:      merlmat.cpp

   Created Time:  2011-08-04 12:46:42

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "merlmat.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"

IMPLEMENT_CREATOR( MerlMat );

// default constructor
MerlMat::MerlMat()
{
	_init();
}
// destructor
MerlMat::~MerlMat()
{
	_release();
}

// register property
void MerlMat::_registerAllProperty()
{
	_registerProperty( "merl" , new MerlProperty( this ) );
}

// initialize default value and register property
void MerlMat::_init()
{
	_registerAllProperty();
}

// get bsdf
Bsdf* MerlMat::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC( Bsdf )( intersect );
	vector<Merl*>::const_iterator it = m_bxdf.begin();
	while( it != m_bxdf.end() )
	{
		bsdf->AddBxdf( *it );
		it++;
	}
	
	return bsdf;
}

// release
void MerlMat::_release()
{
	vector<Merl*>::const_iterator it = m_bxdf.begin();
	while( it != m_bxdf.end() )
	{
		delete *it;
		it++;
	}
}
