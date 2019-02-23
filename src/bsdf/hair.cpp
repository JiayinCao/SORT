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

#include "hair.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"

Spectrum Hair::f( const Vector& wo , const Vector& wi ) const{
    // to be replaced
    if (!SameHemiSphere(wo, wi)) return 0.0f;
    if (!doubleSided && !PointingUp(wo)) return 0.0f;
    return baseColor * INV_PI * AbsCosTheta(wi);
}

Spectrum Hair::sample_f(const Vector& wo, Vector& wi, const BsdfSample& bs, float* pPdf) const {
    return Bxdf::sample_f( wo , wi , bs , pPdf );
}

float Hair::pdf( const Vector& wo , const Vector& wi ) const{
    return Bxdf::pdf( wo , wi );
}
