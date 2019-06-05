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
constexpr float Io[] = {    4.00135,3.67869,3.41946,3.14422,2.90065,2.70883,2.51648,2.35999,2.18996,2.04326,
                            1.90551,1.78215,1.66645,1.57165,1.47358,1.38601,1.30506,1.22461,1.16122,1.10522,
                            1.03522,0.979266,0.927771,0.888074,0.841055,0.792354,0.756729,0.725017,0.69147,0.654355,
                            0.634222,0.599386,0.574827,0.552197,0.529439,0.515637,0.488317,0.467605,0.447081,0.430586,
                            0.421249,0.400906,0.386828,0.37073,0.359457,0.348673,0.332789,0.320507,0.315672,0.303921,
                            0.295139,0.289683,0.275853,0.265854,0.261691,0.253324,0.246592,0.239079,0.237477,0.223207,
                            0.219531,0.211183,0.20457,0.20255,0.196592,0.190717,0.182978,0.180593,0.177309,0.17343,
                            0.167572,0.167867,0.15902,0.155644,0.153431,0.156067,0.149464,0.145845,0.142703,0.139409,
                            0.13455,0.133639,0.12828,0.124672,0.125799,0.120591,0.118215,0.116312,0.114302,0.114966,
                            0.111418,0.108334,0.107044,0.104634,0.102425,0.099655,0.098655,0.0956427,0.093926,0.0931403,
                            0.0916826,0.0922175,0.0893976,0.0872402,0.0860362,0.0835398,0.0798139,0.0799376,0.0777592,0.0783926,
                            0.0764478,0.0756649,0.0753106,0.0723512,0.0720458,0.0712372,0.0703747,0.0685294,0.0680297,0.0666005,
                            0.0659012,0.0632552,0.0641641,0.0621919,0.0634569,0.0603177,0.0606063,0.0591056,0.0587874,0.056618,
                            0.0578634,0.056498,0.0559086,0.0536425,0.0530503,0.0535049,0.0529262,0.0508928,0.0518059,0.0491237,
                            0.0496852,0.048203,0.0489969,0.0469119,0.0479599,0.0466028,0.0454572,0.0457384,0.0453972,0.0445162,
                            0.0417916,0.0438724,0.0418591,0.0401655,0.0412564,0.0410897,0.0398894,0.0384717,0.0395579,0.0381469,
                            0.0395747,0.0390708,0.0386849,0.0369092,0.0373312,0.036083,0.0356972,0.0348813,0.0352278,0.0337435,
                            0.035194,0.0335923,0.0341906,0.034172,0.0324414,0.0324928,0.032876,0.032697,0.0311161,0.0324563,
                            0.0309793,0.0300545,0.030015,0.0304442,0.0298093,0.0298667,0.0294005,0.0305485,0.028564,0.0277717,
                            0.027917,0.0273427,0.0282309,0.027132,0.0269347,0.02691,0.0259039,0.026189,0.0260026,0.0246923,
                            0.0255941,0.025529,0.0249152,0.0253921,0.0244584,0.0248378,0.024623,0.0243495,0.0231759,0.0222915,
                            0.0235275,0.0234334,0.0233913,0.022993,0.0226266,0.0226178,0.0221229,0.0221086,0.0217979,0.0219734,
                            0.0214273,0.0213473,0.0203149,0.0203873,0.0209581,0.0208592,0.0206922,0.0199258,0.0198031,0.0197765,
                            0.0192126,0.0190685,0.0188029,0.0190481,0.0183252,0.018652,0.0182749,0.0177747,0.0176666,0.0173711,
                            0.0182572,0.0179784,0.0173477,0.0172051,0.0168426,0.0174461,0.0173127,0.0174671,0.0169471,0.0165887,
                            0.0178178,0.0166148,0.0160559,0.0165248,0.0156747,0.016016 };
static_assert( sizeof( Io ) / sizeof( float ) == 256 , "Incorrect pre-integrated array size." );

Spectrum Fabric::f( const Vector& wo , const Vector& wi ) const{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto i = (int)(( N / 30.0f ) * 255.0f);
    const auto io = Io[i];

    //const auto theta_o = acos( saturate( CosTheta(wo) ) );
    //const auto theta_i = acos( saturate( CosTheta(wi) ) );
    const auto wh = Normalize( wo + wi );
    return baseColor * pow( 1.0f - SinTheta( wh ) , N ) * AbsCosTheta(wi) / io;
}

Spectrum Fabric::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    return Bxdf::sample_f( wo , wi , bs , pPdf );

    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto r = sort_canonical();
    const auto theta_h = asin( saturate( 1.0f - pow( r , 1.0f / ( N + 1 ) ) ) );
    //const auto theta = 2.0f * theta_h - acos( saturate( CosTheta( wo ) ) );
    const auto phi_h = TWO_PI * sort_canonical();

    const auto wh = SphericalVec( theta_h , phi_h );
    wi = reflect( wo , wh );

    if( pPdf )
        *pPdf = pdf( wo , wi );
    return f( wo , wi );
}

float Fabric::pdf(const Vector& wo, const Vector& wi) const {
    return Bxdf::pdf( wo , wi );

    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    if (SinTheta( wi ) <= 0.0f ) return 0.0f;

    const auto N = ceil(1 + 29 * SQR(1 - roughness));

    const auto wh = Normalize( wo + wi );
    const auto pdf_theta = ( N + 1 ) * pow( 1.0f - SinTheta( wh ) , N );
    const auto pdf_phi = 1.0f;

    return pdf_theta * pdf_phi / ( SinTheta( wi ) * 4.0f * Dot( wo , wh ) );
}