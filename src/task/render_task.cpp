/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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
#include "medium/medium.h"
#include "core/display_mgr.h"

std::atomic<int> g_render_task_cnt = 0;

Render_Task::Render_Task(const Vector2i& ori , const Vector2i& size , const Scene& scene ,
            const char* name , unsigned int priority , const Task::Task_Container& dependencies ) :
            Task( name , priority , dependencies ), m_coord(ori), m_size(size), m_scene(scene){
    m_sampler = std::make_unique<RandomSampler>();
    m_pixelSamples = std::make_unique<PixelSample[]>(g_samplePerPixel);

    ++g_render_task_cnt;
}

void Render_Task::Execute(){
    if(IS_PTR_INVALID(g_integrator))
        return;

    auto camera = m_scene.GetCamera();

    // request samples
    g_integrator->RequestSample( m_sampler.get() , m_pixelSamples.get() , g_samplePerPixel);

    const bool display_server_connected = DisplayManager::GetSingleton().IsDisplayServerConnected();

    const auto total_pixel = m_size.x * m_size.y;
    std::shared_ptr<DisplayTile> display_tile;
    if (display_server_connected) {
        std::shared_ptr<DisplayTile> indicate_tile;
        indicate_tile = std::make_shared<DisplayTile>();
        indicate_tile->x = m_coord.x;
        indicate_tile->y = m_coord.y;
        indicate_tile->w = m_size.x;
        indicate_tile->h = m_size.y;
        indicate_tile->title = g_imageTitle;

        const auto indication_intensity = 0.3f;
        if (g_blenderMode) {
            indicate_tile->m_data[0] = std::make_unique<float[]>(total_pixel * 4);
            auto data = indicate_tile->m_data[0].get();
            memset(data, 0, sizeof(float) * total_pixel * 4);

            for (auto i = 0u; i < indicate_tile->w; ++i) {
                if ((i >> 2) % 2 == 0)
                    continue;

                for (auto c = 0; c < 3; ++c) {
                    data[4 * i + c] = indication_intensity;
                    data[4 * (total_pixel - 1 - i) + c] = indication_intensity;
                }
                data[4 * i + 3] = 1.0f;
                data[4 * (total_pixel - 1 - i) + 3] = 1.0f;
            }
            for (auto i = 0u; i < indicate_tile->h; ++i) {
                if ((i >> 2) % 2 == 0)
                    continue;

                for (auto c = 0; c < 3; ++c) {
                    data[4 * (i * indicate_tile->w) + c] = indication_intensity;
                    data[4 * (i * indicate_tile->w + indicate_tile->w - 1) + c] = indication_intensity;
                }
                data[4 * (i * indicate_tile->w) + 3] = 1.0f;
                data[4 * (i * indicate_tile->w + indicate_tile->w - 1) + 3] = 1.0f;
            }
        }
        else {
            for (auto i = 0u; i < RGBSPECTRUM_SAMPLE; ++i) {
                indicate_tile->m_data[i] = std::make_unique<float[]>(total_pixel);
                auto data = indicate_tile->m_data[i].get();
                memset(data, 0, sizeof(float) * total_pixel);

                for (auto i = 0u; i < indicate_tile->w; ++i) {
                    if ((i >> 2) % 2 == 0)
                        continue;
                    data[i] = indication_intensity;
                    data[total_pixel - 1 - i] = indication_intensity;
                }
                for (auto i = 0u; i < indicate_tile->h; ++i) {
                    if ((i >> 2) % 2 == 0)
                        continue;
                    data[i * indicate_tile->w] = indication_intensity;
                    data[i * indicate_tile->w + indicate_tile->w - 1] = indication_intensity;
                }
            }
        }

        // indicate that we are rendering this tile
        DisplayManager::GetSingleton().QueueDisplayItem(indicate_tile);

        display_tile = std::make_shared<DisplayTile>();
        display_tile->x = m_coord.x;
        display_tile->y = m_coord.y;
        display_tile->w = m_size.x;
        display_tile->h = m_size.y;
        display_tile->title = g_imageTitle;

        if (g_blenderMode) {
            display_tile->m_data[0] = std::make_unique<float[]>(total_pixel * 4);
        } else {
            for (auto i = 0u; i < 3; ++i)
                display_tile->m_data[i] = std::make_unique<float[]>(total_pixel);
        }
    }

    Vector2i rb = m_coord + m_size;
    for( int i = m_coord.y ; i < rb.y ; i++ ){
        for( int j = m_coord.x ; j < rb.x ; j++ ){
            // generate samples to be used later
            g_integrator->GenerateSample( m_sampler.get() , m_pixelSamples.get(), g_samplePerPixel, m_scene );

            // the radiance
            Spectrum radiance;

            auto valid_pixel_cnt = g_samplePerPixel;
            for( unsigned k = 0 ; k < g_samplePerPixel; ++k ){
                // clear managed memory after each pixel
                SORT_CLEAR_MEMPOOL();

                // generate rays
                auto r = camera->GenerateRay( (float)j , (float)i , m_pixelSamples[k] );
                // accumulate the radiance
                auto li = g_integrator->Li( r , m_pixelSamples[k] , m_scene );
                if( g_clammping > 0.0f )
                    li = li.Clamp( 0.0f , g_clammping );
                
                sAssert( li.IsValid() , GENERAL );
                
                if( li.IsValid() )
                    radiance += li;
                else
                    --valid_pixel_cnt;
            }

            if( valid_pixel_cnt > 0 )
                radiance /= (float)valid_pixel_cnt;
            
            // store the pixel
            g_imageSensor->StorePixel( j , i , radiance , *this );

            // update the value if display server is connected
            if (display_server_connected) {
                auto local_i = i - m_coord.y;
                auto local_j = j - m_coord.x;
                
                if (g_blenderMode) {
                    auto local_index = local_j + (m_size.y - 1 - local_i) * m_size.x;
                    display_tile->m_data[0][4 * local_index] = radiance[0];
                    display_tile->m_data[0][4 * local_index + 1] = radiance[1];
                    display_tile->m_data[0][4 * local_index + 2] = radiance[2];
                    display_tile->m_data[0][4 * local_index + 3] = 1.0f;
                } else {
                    auto local_index = local_j + local_i * m_size.x;
                    for (auto i = 0u; i < RGBSPECTRUM_SAMPLE; ++i)
                        display_tile->m_data[i][local_index] = radiance[i];
                }
            }
        }
    }

    // update display server if needed
    if (display_server_connected)
        DisplayManager::GetSingleton().QueueDisplayItem(display_tile);

    // this needs to go away eventaully.
    if( g_integrator->NeedRefreshTile() ){
        auto x_off = m_coord.x / g_tileSize;
        auto y_off = (g_resultResollutionHeight - 1 - m_coord.y ) / g_tileSize ;
        g_imageSensor->FinishTile( x_off, y_off, *this );
    }

    // we are done rendering this task
    --g_render_task_cnt;
}

void PreRender_Task::Execute(){
    g_integrator->PreProcess(m_scene);
}
