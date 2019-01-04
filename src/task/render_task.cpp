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

#include "render_task.h"
#include "integrator/integrator.h"
#include "sampler/sampler.h"
#include "camera/camera.h"
#include "core/globalconfig.h"
#include "core/scene.h"
#include "core/profile.h"
#include "sampler/random.h"

Render_Task::Render_Task(const Vector2i& ori , const Vector2i& size , const Scene& scene ,
            const char* name , unsigned int priority , const Task::Task_Container& dependencies ) : 
            Task( name , priority , dependencies ), m_coord(ori), m_size(size), m_scene(scene){
    m_sampler = std::make_unique<RandomSampler>();
    m_pixelSamples = std::make_unique<PixelSample[]>(g_samplePerPixel);
}

void Render_Task::Execute(){
    if(g_integrator == nullptr )
        return;
    auto camera = m_scene.GetCamera();

    // request samples
    g_integrator->RequestSample( m_sampler.get() , m_pixelSamples.get() , g_samplePerPixel);
    
	Vector2i rb = m_coord + m_size;
    
    unsigned tid = ThreadId();
    for( int i = m_coord.y ; i < rb.y ; i++ ){
        for( int j = m_coord.x ; j < rb.x ; j++ ){
            auto& allocator_mem = GetStaticAllocator();

            // generate samples to be used later
            g_integrator->GenerateSample( m_sampler.get() , m_pixelSamples.get(), g_samplePerPixel, m_scene );
            
            // the radiance
            Spectrum radiance;

            for( unsigned k = 0 ; k < g_samplePerPixel; ++k ){
                // clear managed memory after each pixel
                SORT_CLEAR_MEMPOOL();

                // generate rays
                auto r = camera->GenerateRay( (float)j , (float)i , m_pixelSamples[k] );
                // accumulate the radiance
                radiance += g_integrator->Li( r , m_pixelSamples[k] , m_scene );
            }
            radiance /= (float)g_samplePerPixel;
            
            // store the pixel
            g_imageSensor->StorePixel( j , i , radiance , *this );
        }
    }
    
	if( g_integrator->NeedRefreshTile() ){
		auto x_off = m_coord.x / g_tileSize;
		auto y_off = (g_resultResollutionHeight - 1 - m_coord.y ) / g_tileSize ;
		g_imageSensor->FinishTile( x_off, y_off, *this );
	}
}

void PreRender_Task::Execute(){
    g_integrator->PreProcess(m_scene);
}