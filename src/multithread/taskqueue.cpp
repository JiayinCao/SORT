/*
   FileName:      taskqueue.cpp

   Created Time:  2015-07-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "taskqueue.h"
#include "integrator/integrator.h"
#include "sampler/sampler.h"
#include "camera/camera.h"
#include "output/sortoutput.h"

// instance the singleton with tex manager
DEFINE_SINGLETON(RenderTaskQueue);

// execute the task
void RenderTask::Execute( Integrator* integrator )
{
	// request samples
	integrator->RequestSample( sampler , pixelSamples , samplePerPixel );

	unsigned right = ori_x + width;
	unsigned bottom = ori_y + height;

	unsigned tid = ThreadId();
	for( unsigned i = ori_y ; i < bottom ; i++ )
	{
		for( unsigned j = ori_x ; j < right ; j++ )
		{
			// clear managed memory after each pixel
			MemManager::GetSingleton().ClearMem(tid);

			// generate samples to be used later
			integrator->GenerateSample( sampler , pixelSamples, samplePerPixel , scene );

			// the radiance
			Spectrum radiance;

			for( unsigned p = 0 ; p < camera->GetPassCount() ; ++p )
			{
				for( unsigned k = 0 ; k < samplePerPixel ; ++k )
				{
					// generate rays
					Ray r = camera->GenerateRay( p , (float)j , (float)i , pixelSamples[k] );
					// accumulate the radiance
					radiance += integrator->Li( r , pixelSamples[k] ) * camera->GetPassFilter(p);
				}
			}
			radiance /= (float)samplePerPixel;

			vector<SORTOutput*>::const_iterator it = outputs.begin();
			while( it != outputs.end() )
			{
				(*it)->StorePixel( j , i , radiance , *this );
				++it;
			}
		}
	}

	int h = outputs[0]->m_height;
	int x_off = ori_x / 64;
	int y_off = (h - 1 - ori_y ) / 64 ;
	vector<SORTOutput*>::const_iterator it = outputs.begin();
	while( it != outputs.end() )
	{
		(*it)->FinishTile( x_off , y_off , *this );
		++it;
	}

	taskDone[taskId] = true;
}