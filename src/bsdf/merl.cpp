/*
 * filename :	merl.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "merl.h"
#include "utility/define.h"
#include "managers/memmanager.h"

// default constructor
Merl::Merl( const string& filename )
{
	// initialize default data
	_init();

	// load the brdf data
	_loadBrdf( filename );
}

// destructor
Merl::~Merl()
{
	_release();
}

// initialize default data
void Merl::_init()
{
	m_data = 0;
}

// release data
void Merl::_release()
{
	SAFE_DELETE(m_data);
}

// load brdf data from file
void Merl::_loadBrdf( const string& filename )
{
	m_data = new double[12];
}

// evaluate bxdf
Spectrum Merl::f( const Vector& wo , const Vector& wi ) const
{
	return Spectrum( 1.0f , 0.0 , 0.0f );
}

// sample a direction randomly
Spectrum Merl::Sample_f( const Vector& wo , Vector& wi , float* pdf ) const
{
	return Spectrum();
}

// clone merl bxdf
Merl* Merl::Clone() const
{
	// NOTE , the destructor of 'm' should never be called!!!
	Merl* m = SORT_MALLOC(Merl);
	m->m_data = m_data;
	return m;
}