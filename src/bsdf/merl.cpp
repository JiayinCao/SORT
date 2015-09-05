/*
   FileName:      merl.cpp

   Created Time:  2011-08-04 12:52:36

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "merl.h"
#include "utility/define.h"
#include "managers/memmanager.h"
#include "utility/path.h"
#include "geometry/vector.h"
#include <fstream>
#include "bsdf.h"

// constant to be used in merl
const unsigned Merl::MERL_SAMPLING_RES_THETA_H = 90;
const unsigned Merl::MERL_SAMPLING_RES_THETA_D = 90;
const unsigned Merl::MERL_SAMPLING_RES_PHI_D = 180;
const unsigned Merl::MERL_SAMPLING_COUNT = 1458000;
const double Merl::MERL_RED_SCALE = 0.0006666666666667;
const double Merl::MERL_GREEN_SCALE = 0.000766666666666667;
const double Merl::MERL_BLUE_SCALE = 0.0011066666666666667;

// default constructor
Merl::Merl()
{
	// initialize default data
	_init();
}

// Load data from file
void Merl::LoadData( const string& filename )
{
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
	m_type = BXDF_GLOSSY;
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
	string str = ( filename );

	// try to open the file
	ifstream file( str.c_str() , ios::binary );
	if( false == file.is_open() )
		return;

	int dims[3];
	file.read( (char*)dims , sizeof( int ) * 3 );

	// check dimension
	if( dims[0] != MERL_SAMPLING_RES_THETA_H ||
		dims[1] != MERL_SAMPLING_RES_THETA_D ||
		dims[2] != MERL_SAMPLING_RES_PHI_D )
	{
		file.close();
		return;
	}

	// allocate data
	unsigned trunksize = dims[0] * dims[1] * dims[2];
	unsigned size = 3 * trunksize;
	m_data = new double[size];
	file.read( (char*)m_data , sizeof( double ) * size );

	unsigned offset = 0;
	for( unsigned i = 0 ; i < trunksize ; i++ )
		m_data[offset++] *= MERL_RED_SCALE;
	for( unsigned i = 0 ; i < trunksize ; i++ )
		m_data[offset++] *= MERL_GREEN_SCALE;
	for( unsigned i = 0 ; i < trunksize ; i++ )
		m_data[offset++] *= MERL_BLUE_SCALE;

	file.close();
}

// evaluate bxdf
Spectrum Merl::f( const Vector& Wo , const Vector& Wi ) const
{
	Vector wo = Wo;
	Vector wi = Wi;

	// Compute wh and transform wi to halfangle coordinate system
    Vector wh = wo + wi;
	if( wh.y < 0.0f )
	{
		wh = -wh;
		wi = -wi;
		wo = -wo;
	}
    if (wh.x == 0.f && wh.y == 0.f && wh.z == 0.f)
		return Spectrum (0.f);
    wh = Normalize(wh);

	float whTheta = SphericalTheta(wh);
    float whCosPhi = CosPhi(wh), whSinPhi = SinPhi(wh);
    float whCosTheta = CosTheta(wh), whSinTheta = SinTheta(wh);

	Vector whx( whSinPhi , 0 , -whCosPhi );
	Vector why( whCosPhi * whCosTheta , -whSinTheta , whSinPhi * whCosTheta );
    Vector wd(Dot(wi, whx), Dot(wi, wh), Dot(wi, why));

    // Compute _index_ into measured BRDF tables
    float wdTheta = SphericalTheta(wd), wdPhi = SphericalPhi(wd);
    if (wdPhi > PI) 
		wdPhi -= PI;

    // Compute indices _whThetaIndex_, _wdThetaIndex_, _wdPhiIndex_
    int whThetaIndex = (int)clamp(sqrtf(max(0.f, whTheta * 2.0f * INV_PI )) * MERL_SAMPLING_RES_THETA_H,  0.f, (float)(MERL_SAMPLING_RES_THETA_H-1));
    int wdThetaIndex = (int)clamp(wdTheta * INV_PI * 2.0f * MERL_SAMPLING_RES_THETA_D, 0.f , (float)(MERL_SAMPLING_RES_THETA_D-1));
    int wdPhiIndex = (int)clamp(wdPhi * INV_PI * MERL_SAMPLING_RES_PHI_D, 0.f , (float)(MERL_SAMPLING_RES_PHI_D - 1));

	// calculate the index
    int index = wdPhiIndex + MERL_SAMPLING_RES_PHI_D * (wdThetaIndex + whThetaIndex * MERL_SAMPLING_RES_THETA_D);

	const float r = (float)( m_data[ index ] );
	index += MERL_SAMPLING_COUNT;
	const float g = (float)( m_data[ index ] );
	index += MERL_SAMPLING_COUNT;
	const float b = (float)( m_data[ index ] );

	return Spectrum( r , g , b );
}
