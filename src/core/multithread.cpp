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

#include "multithread.h"
#include "integrator/integrator.h"
#include "sampler/sampler.h"
#include "camera/camera.h"
#include "imagesensor/imagesensor.h"

extern int g_iTileSize;

// execute the task
void RenderTask::Execute( std::shared_ptr<Integrator> integrator )
{
    ImageSensor* is = camera->GetImageSensor();
    if( !is )
        return;
    
    // request samples
    integrator->RequestSample( sampler , pixelSamples , samplePerPixel );
    
	Vector2i rb = ori + size;
    
    unsigned tid = ThreadId();
    for( int i = ori.y ; i < rb.y ; i++ )
    {
        for( int j = ori.x ; j < rb.x ; j++ )
        {
            // clear managed memory after each pixel
            MemManager::GetSingleton().ClearMem(tid);
            
            // generate samples to be used later
            integrator->GenerateSample( sampler , pixelSamples, samplePerPixel , scene );
            
            // the radiance
            Spectrum radiance;

            for( unsigned k = 0 ; k < samplePerPixel ; ++k )
            {
                // generate rays
                Ray r = camera->GenerateRay( (float)j , (float)i , pixelSamples[k] );
                // accumulate the radiance
                radiance += integrator->Li( r , pixelSamples[k] );
            }
            radiance /= (float)samplePerPixel;
            
            // store the pixel
            is->StorePixel( j , i , radiance , *this );
        }
    }
    
	if( integrator->NeedRefreshTile() )
	{
		int x_off = ori.x / g_iTileSize;
		int y_off = (is->GetHeight() - 1 - ori.y ) / g_iTileSize ;
		is->FinishTile( x_off, y_off, *this );
	}
    
    taskDone[taskId] = true;
}
