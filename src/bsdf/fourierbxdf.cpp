/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
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

void FourierBxdfData::LoadData( const std::string& filename )
{
    std::ifstream file( filename.c_str(), std::ios::binary);
    if( !file.is_open() )
        return;
    
    // We assume both of the system and the file are of the same endian, which should be little endian on Intel chip.
    auto ReadFile = [&]( char* data , int sizeInByte )-> bool {
        file.read( data , sizeInByte );
        return sizeInByte == file.gcount();
    };
    
    const char* header = "SCATFUN\x01";
    char file_header[8];
    memset( file_header , 0 , sizeof( file_header ) );
    ReadFile( file_header , 8 );
    if( memcmp( file_header , header , 8 ) != 0 )
        return;
    
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
    bsdfTable.a0 = new float[sqMu];
    bsdfTable.recip = new float[bsdfTable.nMu];
    
    if(!ReadFile( (char*)bsdfTable.mu , bsdfTable.nMu * sizeof(float) ) ||
       !ReadFile( (char*)bsdfTable.cdf , sqMu * sizeof(float) ) ||
       !ReadFile( (char*)offsetAndLength.get() , 2 * sqMu * sizeof(int) ) ||
       !ReadFile( (char*)bsdfTable.a , coeff * sizeof( float ) ) )
        {file.close(); return;}
    
    for( int i = 0 ; i < bsdfTable.nMu * bsdfTable.nMu ; ++i ){
        bsdfTable.aOffset[i] = offsetAndLength[2*i];
        bsdfTable.m[i] = offsetAndLength[2*i+1];
        bsdfTable.a0[i] = ( bsdfTable.m[i] > 0 )?bsdfTable.a[offsetAndLength[2*i]]:0.0f;
    }
    
    bsdfTable.recip[0] = 0.0f;
    for( int i = 1 ; i < bsdfTable.nMu ; ++i )
        bsdfTable.recip[i] = 1.0f / (float) i;
    
    file.close();
}

// evaluate bxdf
Spectrum FourierBxdfData::f( const Vector& wo , const Vector& wi ) const
{
    const float muI = CosTheta( -wi );
    const float muO = CosTheta( wo );
    const float dPhi = CosDPhi( wo , -wi );
    
    int offsetI , offsetO;
    float weightsI[4] , weightsO[4];
    if( !getCatmullRomWeights( muI , offsetI , weightsI ) ||
        !getCatmullRomWeights( muO , offsetO , weightsO ) )
        return 0.0f;
    
    float* ak = SORT_MALLOC_ARRAY(float, bsdfTable.nMax * bsdfTable.nChannels );
    memset( ak , 0 , sizeof( float ) * bsdfTable.nMax * bsdfTable.nChannels );
    int nMax = blendCoefficients( ak , bsdfTable.nChannels , offsetI, offsetO, weightsI, weightsO );
    
    float Y = std::max( 0.0f , fourier( ak , nMax , dPhi ) );
    float scale = ( muI != 0.0f ) ? ( 1 / fabs(muI) ) : 0.0f;
    if( muI * muO > 0.0f ){
        float eta = ( muI > 0.0f ) ? 1 / bsdfTable.eta : bsdfTable.eta;
        scale *= eta * eta;
    }
    
    if( bsdfTable.nChannels == 1 )
        return scale * Y;
    
    float R = fourier( ak + 1 * bsdfTable.nMax , nMax , dPhi );
    float B = fourier( ak + 2 * bsdfTable.nMax , nMax , dPhi );
    float G = 1.39829f * Y - 0.100913f * B - 0.297375f * R;
    return Spectrum( R * scale , G * scale , B * scale ).Clamp( 0.0f , FLT_MAX );
}

Spectrum FourierBxdfData::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const
{
    float muO = CosTheta(wo);
    float pdfMu;
    float muI = sampleCatmullRom2D(bsdfTable.nMu, bsdfTable.nMu, bsdfTable.mu, bsdfTable.mu, bsdfTable.a0, bsdfTable.cdf, muO, bs.u, nullptr, &pdfMu);
    
    int offsetI , offsetO;
    float weightsI[4] , weightsO[4];
    if( !getCatmullRomWeights( muI , offsetI , weightsI ) ||
        !getCatmullRomWeights( muO , offsetO , weightsO ) ){
        *pdf = 0.0f;
        return 0.0f;
    }
    
    float* ak = SORT_MALLOC_ARRAY(float, bsdfTable.nMax * bsdfTable.nChannels );
    memset( ak , 0 , sizeof( float ) * bsdfTable.nMax * bsdfTable.nChannels );
    int nMax = blendCoefficients( ak , bsdfTable.nChannels , offsetI, offsetO, weightsI, weightsO );
    
    float phi, pdfPhi;
    float Y = sampleFourier(ak, bsdfTable.recip, nMax, bs.v, &pdfPhi, &phi);
    *pdf = std::max( 0.0f , pdfPhi * pdfMu );
    
    float sin2ThetaI = std::max( 0.0f , 1.0f - muI * muI );
    float norm = sqrt( sin2ThetaI / SinTheta2(wo));
    if( isinf(norm) ) norm = 0.0f;
    float sinPhi = sin(phi) , cosPhi = cos(phi);
    wi = -Vector3f( norm * ( cosPhi * wo.x - sinPhi * wo.z ),
                    muI ,
                    norm * ( sinPhi * wo.x + cosPhi * wo.z ));
    
    wi.Normalize();
    
    float scale = ( muI != 0.0f ) ? ( 1 / fabs(muI) ) : 0.0f;
    if( muI * muO > 0.0f ){
        float eta = ( muI > 0.0f ) ? 1 / bsdfTable.eta : bsdfTable.eta;
        scale *= eta * eta;
    }
    
    if( bsdfTable.nChannels == 1 )
        return scale * Y;
    
    float R = fourier( ak + 1 * bsdfTable.nMax , nMax , cosPhi );
    float B = fourier( ak + 2 * bsdfTable.nMax , nMax , cosPhi );
    float G = 1.39829f * Y - 0.100913f * B - 0.297375f * R;
    return Spectrum( R * scale , G * scale , B * scale ).Clamp( 0.0f , FLT_MAX );
}

float FourierBxdfData::pdf( const Vector& wo , const Vector& wi ) const
{
    float muI = CosTheta(-wi) , muO = CosTheta(wo);
    float cosPhi = CosDPhi( -wi , wo );
    
    int offsetI , offsetO;
    float weightsI[4] , weightsO[4];
    if( !getCatmullRomWeights( muI , offsetI , weightsI ) ||
        !getCatmullRomWeights( muO , offsetO , weightsO ) )
        return 0.0f;
    
    float* ak = SORT_MALLOC_ARRAY(float, bsdfTable.nMax );
    memset( ak , 0 , sizeof( float ) * bsdfTable.nMax );
    int nMax = blendCoefficients( ak , 1 , offsetI, offsetO, weightsI, weightsO );
    
    float rho = 0.0f;
    for( int o = 0 ; o < 4 ; ++o ){
        if( weightsO[o] == 0 ) continue;
        rho += weightsO[o] * bsdfTable.cdf[ (offsetO + o) * bsdfTable.nMu + bsdfTable.nMu - 1 ] * TWO_PI;
    }

    float Y = fourier(ak, nMax, cosPhi);
    return (rho > 0.0f && Y > 0.0f) ? (Y/rho) : 0.0f;
}

// helper function find interval that wraps the target value
template<typename Predicate>
int FourierBxdfData::findInterval( int cnt , const Predicate& pred ) const
{
    // use binary search to get the offset
    int l = 0 , r = cnt - 1;
    while( l < r ){
        int m = l + (( r - l ) >> 1);
        if( pred(m) )
            l = m + 1;
        else
            r = m;
    }
    return l - 1;
}

// Get CatmullRomWeights
bool FourierBxdfData::getCatmullRomWeights( float x , int& offset , float* weights ) const
{
    if( !(x >= bsdfTable.mu[0] && x <= bsdfTable.mu[bsdfTable.nMu-1] ) )
        return false;
    
    // use binary search to get the offset
    offset = findInterval( bsdfTable.nMu , [&](int i) { return bsdfTable.mu[i] <= x; } ) - 1;
    
    float x1 = bsdfTable.mu[offset+1] , x2 = bsdfTable.mu[offset+2];
    float t = ( x - x1 ) / ( x2 - x1 ) , t2 = t * t, t3 = t2 * t;
    
    weights[0] = weights[3] = 0.0f;
    weights[1] = 2 * t3 - 3 * t2 + 1;
    weights[2] = -2 * t3 + 3 * t2;
    if( offset >= 0 ){
        float w0 = (t3 - 2 * t2 + t) * (x2 - x1) / (x2 - bsdfTable.mu[offset]);
        weights[0] = -w0;
        weights[2] += w0;
    }else{
        float w0 = t3 - 2 * t2 + t;
        weights[1] -= w0;
        weights[2] += w0;
    }
    if( offset < bsdfTable.nMu - 3 ){
        float w3 = (t3 - t2) * (x2 - x1) / (bsdfTable.mu[offset+3] - x1);
        weights[1] -= w3;
        weights[3] = w3;
    }else{
        float w3 = t3 - t2;
        weights[1] -= w3;
        weights[2] += w3;
    }

    return true;
}

// Importance sampling for catmull rom
float FourierBxdfData::sampleCatmullRom2D( int size1 , int size2 , const float* nodes1 , const float* nodes2 , const float* values , const float* cdf ,
                                      float alpha , float u , float* fval , float* pdf ) const
{
    // Determine offset and coefficients for the alpha parameter
    int offset;
    float weights[4];
    if( !getCatmullRomWeights( alpha , offset , weights ) )
        return 0.0f;
    
    // Interpolate between the four table entries
    auto interpolate = [&]( const float* array , int idx ){
        float value = 0.0f;
        for( int i = 0 ; i < 4 ; ++i ){
            if( weights[i] != 0.0f )
                value += array[ ( i + offset ) * size2 + idx ] * weights[i];
        }
        return value;
    };
    
    float maximum = interpolate( cdf , size2 - 1 );
    u *= maximum;
    int idx = findInterval( size2, [&](int i){ return interpolate(cdf,i) <= u; } );
    
    float f0 = interpolate( values , idx );
    float f1 = interpolate( values , idx + 1 );
    float x0 = nodes2[idx] , x1 = nodes2[idx+1];
    float w = x1 - x0;
    float d0 , d1;
    
    u = ( u - interpolate( cdf , idx ) ) / w;
    
    if( idx > 0 )
        d0 = w * ( f1 - interpolate( values , idx - 1 ) ) / ( x1 - nodes2[idx-1] );
    else
        d0 = ( f1 - f0 );
    if( idx < size2 - 2 )
        d1 = w * ( interpolate( values , idx + 2 ) - f0 ) / ( nodes2[idx+2] - x0 );
    else
        d1 = f1 - f0;
    
    // Approximate the intersection by assuming it is linear interpolant
    float t = ( f0 != f1 ) ? ( f0 - sqrt(std::max( 0.0f , f0 * f0 + 2 * u * ( f1 - f0 ) ) ) ) / (f0 - f1 ) : u / f0;
    float a = 0.0f , b = 1.0f, Fhat , fhat;
    while(true){
        if( !( t >= a && t <= b ) )
            t = ( a + b ) * 0.5f;
        
        // Evaluate target function and its derivative in Horner form
        Fhat = t * (f0 + t * (.5f * d0 + t * ((1.f / 3.f) * (-2 * d0 - d1) + f1 - f0 + t * (.25f * (d0 + d1) + .5f * (f0 - f1)))));
        fhat = f0 + t * (d0 + t * (-2 * d0 - d1 + 3 * (f1 - f0) + t * (d0 + d1 + 2 * (f0 - f1))));
        
        if( fabs( Fhat - u ) < 1e-6f || b - a < 1e-6f )
            break;
        
        if( Fhat - u < 0.0f )
            a = t;
        else
            b = t;
        
        t -= (Fhat-u) / fhat;
    }
    
    if( fval ) *fval = fhat;
    if( pdf ) *pdf = fhat / maximum;
    return x0 + w * t;
}

// Fourier interpolation
float FourierBxdfData::fourier( const float* ak , int m , double cosPhi ) const
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
    return (float)value;
}

// Importance sampling for fourier interpolation
// Refer these two wiki pages for further detail:
// Bisection method :   https://en.wikipedia.org/wiki/Bisection_method
// Newton method :      https://en.wikipedia.org/wiki/Newton%27s_method
float FourierBxdfData::sampleFourier( const float* ak , const float* recip , int m , float u , float* pdf , float* phiptr ) const
{
    bool flip = u >= 0.5f;
    if( flip ) u = 2.0f * ( 1.0f - u );
    else u *= 2.0f;
    
    // Bisection method
    double l = 0.0f , r = PI, phi = 0.5f * PI;
    double F, f;
    while( true ){
        double cosPhi = cos(phi);
        double sinPhi = sqrt( std::max( 0.0 , 1.0f - cosPhi * cosPhi ) );
        double cosPrevPhi = cosPhi , cosCurPhi = 1.0;
        double sinPrevPhi = -sinPhi , sinCurPhi = 0.0;
        
        F = ak[0] * phi;
        f = ak[0];
        for( int i = 1 ; i < m ; ++i ){
            double cosNextPhi = 2.0 * cosCurPhi * cosPhi - cosPrevPhi;
            double sinNextPhi = 2.0 * sinCurPhi * cosPhi - sinPrevPhi;
            cosPrevPhi = cosCurPhi;
            cosCurPhi = cosNextPhi;
            sinPrevPhi = sinCurPhi;
            sinCurPhi = sinNextPhi;
            
            F += ak[i] * sinCurPhi * recip[i];
            f += ak[i] * cosCurPhi;
        }
        F -= u * ak[0] * PI;
        
        if( F > 0.0 ) r = phi;
        else l = phi;
        
        if( fabs(F) < 1e-6f || r - l < 1e-6f )
            break;
        
        // Newton method
        phi -= F / f;
        
        if( !(phi > l && phi < r ) )
            phi = ( l + r ) * 0.5f;
    }
    
    if( flip ) phi = TWO_PI - phi;
    if( pdf ) *pdf = (float) ( INV_TWOPI * f / ak[0] );
    *phiptr = (float)phi;
    
    return (float)f;
}

// helper functio to blend coefficients for fourier
int FourierBxdfData::blendCoefficients( float* ak , int channel , int offsetI , int offsetO , float* weightsI , float* weightsO ) const
{
    int nMax = 0;
    for( int i = 0 ; i < 4 ; ++i ){
        for( int j = 0 ; j < 4 ; ++j ){
            float w = weightsI[j] * weightsO[i];
            if( w != 0.0f ){
                int m;
                float* a = bsdfTable.GetAk(offsetI + j , offsetO + i, &m );
                nMax = std::max( nMax , m );
                for( int c = 0 ; c < channel ; ++c ){
                    for( int k = 0 ; k < m ; ++k )
                        ak[ c * bsdfTable.nMax + k ] += w * a[ c * m + k ];
                }
            }
        }
    }
    return nMax;
}
