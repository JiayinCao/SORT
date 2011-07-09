/*
 * filename :	merl.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "merl.h"
#include "utility/define.h"
#include "managers/memmanager.h"
#include "utility/path.h"
#include "geometry/vector.h"
#include <fstream>
#include <math.h>

// constant to be used in merl
const unsigned Merl::MERL_SAMPLING_RES_THETA_H = 90;
const unsigned Merl::MERL_SAMPLING_RES_THETA_D = 90;
const unsigned Merl::MERL_SAMPLING_RES_PHI_D = 360;
const double Merl::MERL_RED_SCALE = 0.0006666666666667;
const double Merl::MERL_GREEN_SCALE = 0.000766666666666667;
const double Merl::MERL_BLUE_SCALE = 0.0011066666666666667;

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
	// get full path
	string str = GetFullPath( filename );

	// try to open the file
	ifstream file( str.c_str() , ios::binary );
	if( false == file.is_open() )
		return;

	int dims[3];
	file.read( (char*)dims , sizeof( int ) * 3 );

	// check dimension
	if( dims[0] != MERL_SAMPLING_RES_THETA_H ||
		dims[1] != MERL_SAMPLING_RES_THETA_D ||
		dims[2] != MERL_SAMPLING_RES_PHI_D / 2 )
	{
		file.close();
		return;
	}

	// allocate data
	unsigned size = 3 * dims[0] * dims[1] * dims[2];
	m_data = new double[size];
	file.read( (char*)m_data , sizeof( double ) * size );

	file.close();
}

// evaluate bxdf
Spectrum Merl::f( const Vector& wo , const Vector& wi ) const
{
	float theta_half , phi_half , phi_diff , theta_diff;
	_std_coords_to_half_diff_coords( wi , wo , theta_half , phi_half , theta_diff , phi_diff );

	// find index
	const int index =	phi_diff_index( phi_diff ) +
						theta_diff_index( theta_diff ) * MERL_SAMPLING_RES_PHI_D / 2 +
						theta_half_index( theta_half ) * MERL_SAMPLING_RES_PHI_D / 2 * MERL_SAMPLING_RES_THETA_D;

	const float r = (float)( m_data[ index ] * MERL_RED_SCALE );
	const float g = (float)( m_data[ index + MERL_SAMPLING_RES_THETA_H * MERL_SAMPLING_RES_THETA_D * MERL_SAMPLING_RES_PHI_D / 2 ] * MERL_GREEN_SCALE );
	const float b = (float)( m_data[ index + MERL_SAMPLING_RES_THETA_H * MERL_SAMPLING_RES_THETA_D * MERL_SAMPLING_RES_PHI_D ] * MERL_BLUE_SCALE );

	return Spectrum( r , g , b );
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

// transform coordinate
void Merl::_std_coords_to_half_diff_coords( const Vector& wi , const Vector& wo , 
										   float& theta_half , float& phi_half , float& theta_diff , float& phi_diff ) const
{
	Vector half = ( wi + wo ) * 0.5f;
	half.Normalize();

	theta_half = acos( half[1] );
	phi_half = atan2( half[2] , half[0] );

	Vector bi_normal( 0.0f , 1.0f , 0.0f );
	Vector normal( 0.0f , 0.0f , 1.0f );
	Vector diff = wi;

	normal.Rotate( diff , -phi_half );
	bi_normal.Rotate( diff , -theta_half );

	theta_diff = acos( diff[1] );
	phi_diff = atan2( diff[2] , diff[0] );
}

// lookup theta_half index
int Merl::theta_half_index( float theta_half ) const
{
	if( theta_half <= 0.0 )
		return 0;

	float theta_half_deg = ( ( theta_half * 2.0f / PI ) * MERL_SAMPLING_RES_THETA_H );
	theta_half_deg *= MERL_SAMPLING_RES_THETA_H;
	theta_half_deg = sqrt( theta_half_deg );
	int ret_val = (int)theta_half_deg;

	if( ret_val < 0 )
		return 0;
	if( ret_val >= MERL_SAMPLING_RES_THETA_H )
		return MERL_SAMPLING_RES_THETA_H - 1;

	return ret_val;
}

// lookup theta_diff index
int	Merl::theta_diff_index( float theta_diff ) const
{
	int temp = int( ( theta_diff * 0.5f / PI ) * MERL_SAMPLING_RES_THETA_D );

	if( temp < 0 )
		return 0;
	if( temp >= MERL_SAMPLING_RES_THETA_D )
		return MERL_SAMPLING_RES_THETA_D - 1;

	return temp;
}

// lookup phi_diff index
int Merl::phi_diff_index( float phi_diff ) const
{
	if( phi_diff < 0.0f )
		phi_diff += PI;

	int temp = int( phi_diff / PI * MERL_SAMPLING_RES_PHI_D * 0.5f );

	if( temp < 0 )
		return 0;
	if( temp >= MERL_SAMPLING_RES_PHI_D * 0.5f )
		return (int)(MERL_SAMPLING_RES_PHI_D * 0.5f - 1);

	return temp;
}
