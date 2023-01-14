/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "hdrskylight.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"

bool HdrSkyLight::Le( const Ray& ray , SurfaceInteraction* intersect , Spectrum& radiance ) const{
    if( intersect && intersect->t != FLT_MAX )
        return false;

    radiance = sky.Evaluate( m_light2world.GetInversed().TransformVector(ray.m_Dir) ) * intensity;
    return true;
}

float HdrSkyLight::Pdf( const Point& p , const Vector& wi ) const{
    return sky.Pdf( m_light2world.GetInversed().TransformVector(wi) );
}

bool HdrSkyLight::LoadHdrImage(const std::string& filepath){
    return sky.Load(filepath);
}