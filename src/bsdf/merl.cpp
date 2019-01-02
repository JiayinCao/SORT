/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <string.h>
#include <fstream>
#include "merl.h"
#include "core/define.h"
#include "managers/memmanager.h"
#include "core/path.h"
#include "math/vector3.h"
#include "bsdf.h"

// constant to be used in merl
static const unsigned MERL_SAMPLING_RES_THETA_H = 90;
static const unsigned MERL_SAMPLING_RES_THETA_D = 90;
static const unsigned MERL_SAMPLING_RES_PHI_D = 180;
static const unsigned MERL_SAMPLING_COUNT = 1458000;
static const double MERL_RED_SCALE = 0.0006666666666667;
static const double MERL_GREEN_SCALE = 0.000766666666666667;
static const double MERL_BLUE_SCALE = 0.0011066666666666667;

// Load data from file
void MerlData::LoadData( const std::string& filename )
{
    // get full path
    auto str = ( filename );
    
    // try to open the file
    std::ifstream file( str.c_str() , std::ios::binary );
    if( false == file.is_open() )
        return;
    
    unsigned int dims[3];
    file.read( (char*)dims , sizeof(unsigned int) * 3 );
    
    // check dimension
    if( dims[0] != MERL_SAMPLING_RES_THETA_H ||
       dims[1] != MERL_SAMPLING_RES_THETA_D ||
       dims[2] != MERL_SAMPLING_RES_PHI_D ){
        file.close();
        return;
    }
    
    // allocate data
    auto trunksize = dims[0] * dims[1] * dims[2];
    auto size = 3u * trunksize;
    m_data = std::make_unique<double[]>(size);
    file.read( (char*)m_data.get() , sizeof( double ) * size );
    
    unsigned offset = 0;
    for(auto i = 0u ; i < trunksize ; i++ )
        m_data[offset++] *= MERL_RED_SCALE;
    for(auto i = 0u ; i < trunksize ; i++ )
        m_data[offset++] *= MERL_GREEN_SCALE;
    for(auto i = 0u ; i < trunksize ; i++ )
        m_data[offset++] *= MERL_BLUE_SCALE;
    
    file.close();
}

// evaluate bxdf
Spectrum MerlData::f( const Vector& Wo , const Vector& Wi ) const
{
    auto wo = Wo;
    auto wi = Wi;

    // ignore reflection at the back face
    if( wo.y <= 0.0f )
        return 0.0f;
	
	// Compute wh and transform wi to halfangle coordinate system
    auto wh = wo + wi;
	if( wh.y < 0.0f ){
		wh = -wh;
		wi = -wi;
		wo = -wo;
	}
    if (wh.x == 0.f && wh.y == 0.f && wh.z == 0.f)
		return Spectrum (0.f);
    wh = Normalize(wh);

    auto whTheta = SphericalTheta(wh);
    auto whCosPhi = CosPhi(wh), whSinPhi = SinPhi(wh);
    auto whCosTheta = CosTheta(wh), whSinTheta = SinTheta(wh);

	Vector whx( whSinPhi , 0 , -whCosPhi );
	Vector why( whCosPhi * whCosTheta , -whSinTheta , whSinPhi * whCosTheta );
    Vector wd(Dot(wi, whx), Dot(wi, wh), Dot(wi, why));

    // Compute _index_ into measured BRDF tables
    auto wdTheta = SphericalTheta(wd), wdPhi = SphericalPhi(wd);
    if (wdPhi > PI) 
		wdPhi -= PI;

    // Compute indices _whThetaIndex_, _wdThetaIndex_, _wdPhiIndex_
    auto whThetaIndex = (int)clamp(sqrtf(std::max(0.f, whTheta * 2.0f * INV_PI )) * MERL_SAMPLING_RES_THETA_H,  0.f, (float)(MERL_SAMPLING_RES_THETA_H-1));
    auto wdThetaIndex = (int)clamp(wdTheta * INV_PI * 2.0f * MERL_SAMPLING_RES_THETA_D, 0.f , (float)(MERL_SAMPLING_RES_THETA_D-1));
    auto wdPhiIndex = (int)clamp(wdPhi * INV_PI * MERL_SAMPLING_RES_PHI_D, 0.f , (float)(MERL_SAMPLING_RES_PHI_D - 1));

	// calculate the index
    auto index = wdPhiIndex + MERL_SAMPLING_RES_PHI_D * (wdThetaIndex + whThetaIndex * MERL_SAMPLING_RES_THETA_D);

	const auto r = (float)( m_data[ index ] );
	index += MERL_SAMPLING_COUNT;
	const auto g = (float)( m_data[ index ] );
	index += MERL_SAMPLING_COUNT;
	const auto b = (float)( m_data[ index ] );

	return Spectrum( r , g , b );
}
