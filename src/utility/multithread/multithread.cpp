/*
 FileName:      multithread.cpp
 
 Created Time:  2015-07-13
 
 Auther:        Cao Jiayin
 
 Email:         soraytrace@hotmail.com
 
 Location:      China, Shanghai
 
 Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
 'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
 modify or publish the source code. It's cross platform. You could compile the source code in
 linux and windows , g++ or visual studio 2008 is required.
 */

#include "multithread.h"
#include "integrator/integrator.h"
#include "sampler/sampler.h"
#include "camera/camera.h"
#include "imagesensor/imagesensor.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(RenderTaskQueue);

extern int g_iTileSize;

// execute the task
void RenderTask::Execute( Integrator* integrator )
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