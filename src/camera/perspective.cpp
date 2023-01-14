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

#include "perspective.h"
#include "texture/rendertarget.h"
#include <math.h>
#include "core/sassert.h"
#include "sampler/sample.h"
#include "core/samplemethod.h"
#include "light/light.h"

void PerspectiveCamera::PreProcess()
{
    const float w = (float)m_image_width;
    const float h = (float)m_image_height;
    float aspect = w/h * m_aspectRatioW/m_aspectRatioH;

    float yScale = 1.0f / tan( m_fov * 0.5f );
    float xScale = yScale / aspect;

    // handle different sensor size for blender
    m_imagePlaneDist = yScale * h * 0.5f;
    if( m_sensorW && m_sensorH )
    {
        if( m_aspectFit == 1 || ( m_aspectFit != 2 && aspect > 1 )) // horizontal fit
        {
            xScale = 1.0f / tan( m_fov * 0.5f );
            yScale = xScale * aspect;
            m_imagePlaneDist = xScale * w * 0.5f;
        }else if( m_aspectFit == 2 || ( m_aspectFit != 1 && aspect < 1 ))   // vertical fit
        {
            yScale = 1.0f / tan( m_fov * 0.5f );
            xScale = yScale / aspect;
            m_imagePlaneDist = yScale * h * 0.5f;
        }
    }

    // update focal distance every time
    m_focalDistance = ( m_target - m_eye ).Length();
    m_forward = ( m_target - m_eye ) / m_focalDistance;

    // clip space to raster space
    m_clipToRaster = Scale( w , h , 1.0f ) * Scale( 0.5f , -0.5f , 1.0f ) * Translate( 1.0f , -1.0f , 0.0f ) ;
    m_cameraToClip = Perspective(xScale, yScale);
    m_cameraToRaster = m_clipToRaster * m_cameraToClip;
    m_worldToCamera = ViewLookat( m_eye , m_forward , m_up );
    m_worldToRaster = m_cameraToRaster * m_worldToCamera;
    m_inverseApartureSize = (m_lensRadius==0)? 1.0f : (1.0f / ( m_lensRadius * m_lensRadius * PI));
}

// generate ray
Ray PerspectiveCamera::GenerateRay( float x , float y , const PixelSample& ps ) const{
    const Point rastP( x + ps.img_u , y + ps.img_v , 0.0f );
    Vector view_dir = m_cameraToRaster.invMatrix.TransformPoint( rastP );

    Ray r;
    r.m_Dir = view_dir.Normalize();

    // Handle DOF camera ray adaption
    if( m_lensRadius != 0 )
    {
        Point target = r(m_focalDistance / view_dir.z);

        float s , t;
        UniformSampleDisk( ps.dof_u , ps.dof_v , s , t );

        r.m_Ori.x = s * m_lensRadius;
        r.m_Ori.y = t * m_lensRadius;
        r.m_Dir = normalize( target - r.m_Ori );
    }

    // transform the ray from camera space to world space
    r = m_worldToCamera.invMatrix( r );

    // calculate the pdf for camera ray
    const float cosAtCamera = dot( m_forward , r.m_Dir );
    const float imagePointToCameraDist = m_imagePlaneDist / cosAtCamera;
    const float imageToSolidAngleFactor = imagePointToCameraDist * imagePointToCameraDist / cosAtCamera;

    // the pdf of the ray
    // combination of two pdfs, sampling a point on the apature and a direction from the point
    r.m_fPdfW = imageToSolidAngleFactor;
    r.m_fPdfA = m_inverseApartureSize;

    // importance of the ray
    r.m_we = r.m_fPdfW * r.m_fPdfA / cosAtCamera;

    // cos at camera
    r.m_fCosAtCamera = cosAtCamera;

    return r;
}

// get camera coordinate according to a view direction in world space
Vector2i PerspectiveCamera::GetScreenCoord( const SurfaceInteraction& inter, float* pdfw, float* pdfa, float& cosAtCamera , Spectrum* we ,
                                            Point* eyeP , Visibility* visibility, RenderContext& rc) const{
    const auto delta = 0.001f;
    Vector dir = inter.intersect - m_eye;
    const auto len = dir.Length();
    dir = dir / len;

    // get view space dir
    Point rastP = m_worldToRaster.TransformPoint( inter.intersect );

    // Handle DOF camera ray adaption
    if( m_lensRadius != 0.0f ){
        Point view_target = m_worldToCamera.TransformPoint( inter.intersect );

        float s , t;
        UniformSampleDisk( sort_rand<float>(rc) , sort_rand<float>(rc) , s , t );

        Ray shadow_ray;
        shadow_ray.m_Ori = Point( s , t , 0.0f ) * m_lensRadius;
        shadow_ray.m_Dir = view_target - shadow_ray.m_Ori;
        shadow_ray.m_fMax = shadow_ray.m_Dir.Length();
        shadow_ray.m_Dir /= shadow_ray.m_fMax;
        shadow_ray.m_fMax -= delta;
        shadow_ray.m_fMin += delta;

        Point view_focal_target = shadow_ray( m_focalDistance / shadow_ray.m_Dir.z );
        rastP = m_cameraToRaster.TransformPoint( view_focal_target );

        shadow_ray = m_worldToCamera.invMatrix( shadow_ray );
        visibility->ray = shadow_ray ;

        if( eyeP )
            *eyeP = shadow_ray.m_Ori;
    }else{
        visibility->ray = Ray( inter.intersect , -dir , 0 , delta , len - delta );
        if( eyeP )
            *eyeP = m_eye;
    }

    // calculate the pdf for camera ray
    cosAtCamera = dot( m_forward , dir );
    const float imagePointToCameraDist = m_imagePlaneDist / cosAtCamera;
    const float imageToSolidAngleFactor = imagePointToCameraDist * imagePointToCameraDist / cosAtCamera;

    if( pdfw )
        *pdfw = imageToSolidAngleFactor;
    if( pdfa )
        *pdfa = m_inverseApartureSize;
    if( we )
        *we = imageToSolidAngleFactor * m_inverseApartureSize / cosAtCamera;

    return Vector2i( (int)rastP.x , (int)rastP.y );
}
