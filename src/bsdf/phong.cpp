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
#include "phong.h"
#include "bsdf.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"

Spectrum Phong::f( const Vector& wo , const Vector& wi ) const
{
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    
    Spectrum ret = D * INV_PI;
    if (!S.IsBlack()) {
        const float alpha = SatDot(wi, reflect(wo));
        if (alpha > 0.0f)
            ret += S * (power + 2) * pow(alpha, power) * INV_PI;
    }
    return ret;
}

Spectrum Phong::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pPdf ) const{
    // To be implemented
    return Bxdf::sample_f(wo, wi, bs, pPdf);
}

float Phong::pdf( const Vector& wo , const Vector& wi ) const{
    // To be implemented
    return Bxdf::pdf(wo, wi);
}
