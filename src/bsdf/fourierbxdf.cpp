/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header file
#include "fourierbxdf.h"
#include "bsdf.h"
#include "managers/memmanager.h"
#include "utility/samplemethod.h"
#include <fstream>

// constructor
FourierBxdf::FourierBxdf()
{
	m_type = BXDF_GLOSSY;
}

void FourierBxdf::LoadData( const string& filename )
{
    ifstream file( filename.c_str() );
    if( !file.is_open() )
        return;
    
    const char* header = "SCATFUN\x01";
    char file_header[8];
    memset( file_header , 0 , sizeof( file_header ) );
    file.read( file_header , 8 );
    if( memcmp( file_header , header , 8 ) != 0 )
        return;
    
    // We assume both of the system and the file are of the same endian, which should be little endian on Intel chip.
    auto ReadFile = [&]( char* data , int sizeInByte )-> bool {
        file.read( data , sizeInByte );
        return sizeInByte == file.gcount();
    };
    
    int flags = 0, coeff = 0, unused[4];
    std::unique_ptr<int[]> offsetAndLength;
    
    if( !ReadFile( (char*)&flags , 4 ) || flags != 1 ) {file.close(); return;}
    if( !ReadFile( (char*)&bsdfTable.nMu , 4 ) || bsdfTable.nMu <= 1 ) {file.close(); return;}
    if( !ReadFile( (char*)&coeff , 4 ) || coeff <= 0 ) {file.close(); return;}
    if( !ReadFile( (char*)&bsdfTable.nMax , 4 ) || bsdfTable.nMax <= 0 ) {file.close(); return;}
    if( !ReadFile( (char*)&bsdfTable.nChannels , 4 ) || (bsdfTable.nChannels != 1 && bsdfTable.nChannels != 3) ) {file.close(); return;}
    if( !ReadFile( (char*)unused , 16 ) ) {file.close(); return;}
    if( !ReadFile( (char*)&bsdfTable.eta , 4 ) ) {file.close(); return;}
    if( !ReadFile( (char*)unused , 16 ) ) {file.close(); return;}
    
    const int sqMu = bsdfTable.nMu * bsdfTable.nMu;
    bsdfTable.mu = new float[bsdfTable.nMu];
    bsdfTable.cdf = new float[sqMu];
    offsetAndLength.reset( new int[sqMu * 2] );
    bsdfTable.m = new int[sqMu];
    bsdfTable.aOffset = new int[sqMu];
    bsdfTable.a = new float[coeff];
    
    if(!ReadFile( (char*)bsdfTable.mu , bsdfTable.nMu * sizeof(float) ) ||
       !ReadFile( (char*)bsdfTable.cdf , sqMu * sizeof(float) ) ||
       !ReadFile( (char*)offsetAndLength.get() , 2 * sqMu * sizeof(int) ) ||
       !ReadFile( (char*)bsdfTable.a , coeff * sizeof( int ) ) )
        {file.close(); return;}
    
    for( int i = 0 ; i < bsdfTable.nMu * bsdfTable.nMu ; ++i ){
        bsdfTable.aOffset[i] = offsetAndLength[2*i];
        bsdfTable.m[i] = offsetAndLength[2*i+1];
    }
    
    file.close();
}

// evaluate bxdf
Spectrum FourierBxdf::f( const Vector& wo , const Vector& wi ) const
{
    const float muI = CosTheta( -wi );
    const float muO = CosTheta( wo );
    const float dPhi = CosDPhi( wo , -wi );
    
    int offsetI , offsetO;
    float weightsI[1] , weights[1];
    GetWeightAndOffset( muI , offsetI , weights );
    GetWeightAndOffset( muO , offsetO , weights );
    
    int m;
    float* ak = bsdfTable.GetAk(offsetI, offsetO, &m );
    
    float Y = max( 0.0f , Fourier( ak , m , dPhi ) );
    float scale = ( muI != 0.0f ) ? ( 1 / fabs(muI) ) : 0.0f;
    if( muI * muO > 0.0f ){
        float eta = ( muI > 0.0f ) ? 1 / bsdfTable.eta : bsdfTable.eta;
        scale *= eta * eta;
    }
    
    if( bsdfTable.nChannels == 1 ){
        return scale * Y;
    }
    
    float R = Fourier( ak + 1 * m , m , dPhi );
    float B = Fourier( ak + 2 * m , m , dPhi );
    float G = 1.39829f * Y - 0.100913f * B - 0.297375f * R;
    return Spectrum( R * scale , G * scale , B * scale ).Clamp( 0.0f , FLT_MAX );
}

Spectrum FourierBxdf::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
    wi = UniformSampleSphere( bs.u , bs.v );
    if( pdf ) *pdf = Pdf( wo , wi );
    return f( wo , wi );
}

float FourierBxdf::Pdf( const Vector& wo , const Vector& wi ) const
{
    return UniformSpherePdf();
}

// Get weight and offset.
void FourierBxdf::GetWeightAndOffset( float costheta , int& offset , float* weight ) const
{
    if( costheta < bsdfTable.mu[0] || costheta > bsdfTable.mu[bsdfTable.nMu-1] ){
        *weight = 0;
        offset =0;
        return;
    }else if( costheta == bsdfTable.mu[bsdfTable.nMu-1] ){
        *weight = 1.0f;
        offset = bsdfTable.nMu - 1;
        return;
    }
    
    // box filter is used temporarily
    *weight = 1.0f;
    
    // use binary search to get the offset
    int l = 0 , r = bsdfTable.nMu - 1;
    while( l < r ){
        int m = l + (( r - l ) >> 1);
        if( bsdfTable.mu[m] <= costheta )
            l = m + 1;
        else
            r = m;
    }
    offset = l - 1;
}

// Fourier interpolation
float FourierBxdf::Fourier( const float* ak , int m , double cosPhi ) const
{
    // cos( K * phi ) = 2.0 * cos( (K-1) * phi ) cos( phi ) - cos( (K-2) * Phi );
    double value = 0.0;
    double cosKMinusOnePhi = cosPhi;
    double cosKPhi = 1.0;
    for( int i = 0 ; i < m ; ++i ){
        value += cosKPhi * ak[i];
        double cosKPlusPhi = 2.0 * cosPhi * cosKPhi - cosKMinusOnePhi;
        cosKMinusOnePhi = cosKPhi;
        cosKPhi = cosKPlusPhi;
    }
    return value;
}
