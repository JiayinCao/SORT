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

#include "fabric.h"
#include "core/samplemethod.h"
#include "core/rand.h"

// This array is a pre-integrated array for BRDF normalization using Monte Carlo estimation.
constexpr float Io[] = {3.98501,3.84697,3.6726,3.55824,3.37832,3.28126,3.16199,3.04171,2.95213,2.82915,
                        2.7315,2.6731,2.58954,2.49541,2.4235,2.33565,2.30792,2.2241,2.20148,2.11513,
                        2.0508,1.98957,1.97096,1.87003,1.83256,1.81702,1.73804,1.72203,1.66355,1.63618,
                        1.5985,1.5426,1.50964,1.49772,1.49084,1.45776,1.40488,1.40703,1.36712,1.35675,
                        1.32253,1.27157,1.24514,1.22936,1.20319,1.2281,1.19918,1.15746,1.13127,1.11603,
                        1.12735,1.08645,1.08233,1.06369,1.03899,1.01051,1.01018,1.00472,0.981444,0.950649,
                        0.943421,0.926088,0.938485,0.94161,0.9227,0.89971,0.873851,0.88891,0.821158,0.888289,
                        0.831082,0.812582,0.848938,0.818534,0.819031,0.773401,0.779377,0.80079,0.75268,0.768532,
                        0.758169,0.763209,0.725662,0.724758,0.708668,0.712982,0.709354,0.71069,0.69708,0.690654,
                        0.657391,0.660246,0.664716,0.6544,0.66314,0.64592,0.651248,0.629081,0.634817,0.674018,
                        0.5978,0.613056,0.621613,0.616062,0.586168,0.595236,0.565388,0.58947,0.581611,0.55601,
                        0.572235,0.575748,0.56143,0.538275,0.557155,0.570488,0.527676,0.551316,0.524266,0.527354,
                        0.543459,0.52571,0.505461,0.507449,0.518117,0.514398,0.495277,0.508286,0.500297,0.495421,
                        0.456402,0.477888,0.487695,0.481802,0.458948,0.47914,0.463228,0.466128,0.460427,0.461106,
                        0.479147,0.446175,0.454795,0.447623,0.440882,0.440266,0.438626,0.431301,0.43025,0.447339,
                        0.43053,0.430452,0.425074,0.406429,0.408611,0.412072,0.428056,0.417204,0.416537,0.410993,
                        0.379515,0.397234,0.384606,0.417766,0.381724,0.398866,0.396648,0.368884,0.373359,0.382462,
                        0.379819,0.377159,0.380297,0.365401,0.374218,0.36786,0.366918,0.366154,0.37907,0.362543,
                        0.362077,0.36047,0.360974,0.353432,0.366818,0.338255,0.339231,0.337983,0.354791,0.348727,
                        0.351333,0.336428,0.33591,0.332624,0.32742,0.331235,0.320253,0.343881,0.334116,0.324674,
                        0.335776,0.31595,0.326427,0.30299,0.329804,0.31574,0.310134,0.322293,0.311979,0.306242,
                        0.309336,0.309399,0.307641,0.318602,0.305981,0.31523,0.304206,0.287219,0.300125,0.286151,
                        0.291958,0.305274,0.292858,0.289326,0.31187,0.278387,0.290809,0.277442,0.274947,0.276678,
                        0.288231,0.26525,0.266614,0.279136,0.284396,0.277178,0.266645,0.28288,0.268891,0.271405,
                        0.275313,0.274846,0.273302,0.276798,0.255903,0.269891,0.259999,0.254329,0.256185,0.263762,
                        0.260831,0.250175,0.266615,0.266225,0.264783,0.248885 };
// The exact algorithm to generate the above data is as follow
/*
    for( int i = 0 ; i < 256 ; ++i ){
        if( i % 10 == 0 )
            std::cout<<std::endl;

        const int N = 10000;
        float n = i / 255.0f * 30.0f;
        float sum = 0;
        for( int k = 0 ; k < N ; ++k ){
            auto r = sort_canonical() * HALF_PI;
            sum += pow( 1.0f - sin( r * 0.5f ) , n ) * cos( r );
        }
        sum *= TWO_PI / N;
        std::cout<<sum<<",";
    }
    std::cout<<std::endl;
*/

static_assert( sizeof( Io ) / sizeof( float ) == 256 , "Incorrect pre-integrated array size." );

Spectrum Fabric::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto i = (int)(( N / 30.0f ) * 255.0f);
    const auto io = Io[i];

    const auto h = Normalize( wo + wi );
    return baseColor * pow( 1.0f - fabs(h.x) , N ) * AbsCosTheta(wi) / io;
}

Spectrum Fabric::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));
    const auto sign = sort_canonical() > 0.5f ? 1.0f : -1.0f;
    const auto sin_theta_h = sign * saturate( 1.0f - pow( sort_canonical() , 1.0f / ( N + 1 ) ) );
    const auto cos_theta_h = sqrt( saturate( 1.0f - SQR( sin_theta_h ) ) );
    const auto phi_h = PI * sort_canonical();

    const auto wh = Vector3f( sin_theta_h , cos_theta_h * sin( phi_h ) , cos_theta_h * cos( phi_h ) );
    wi = reflect( wo , wh );

    if( pPdf )
        *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float Fabric::pdf(const Vector& wo, const Vector& wi) const {
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto wh = Normalize( wo + wi );
    const auto pdf_h = ( N + 1 ) * pow( 1.0f - fabs(wh.x) , N ) * INV_TWOPI;
    return pdf_h / ( 4.0f * Dot( wo , wh ) );
}