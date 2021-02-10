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

#include <regex>
#include <marl/defer.h>
#include <marl/event.h>
#include <marl/waitgroup.h>
#include "image_evaluation.h"
#include "core/display_mgr.h"
#include "stream/fstream.h"
#include "core/strid.h"
#include "material/matmanager.h"
#include "core/timer.h"
#include "sampler/random.h"
#include "core/parse_args.h"

SORT_STATS_DEFINE_COUNTER(sPreprocessingTimeMS)
SORT_STATS_DEFINE_COUNTER(sRenderingTimeMS)
SORT_STATS_DEFINE_COUNTER(sSamplePerPixel)
SORT_STATS_DEFINE_COUNTER(sThreadCnt)

SORT_STATS_TIME("Performance", "Acceleration Structure Construction", sPreprocessingTimeMS);
SORT_STATS_TIME("Performance", "Rendering Time", sRenderingTimeMS);
SORT_STATS_AVG_RAY_SECOND("Performance", "Number of rays per second", sRayCount, sRenderingTimeMS);
SORT_STATS_COUNTER("Statistics", "Sample per Pixel", sSamplePerPixel);
SORT_STATS_COUNTER("Performance", "Worker thread number", sThreadCnt);

static constexpr unsigned int GLOBAL_CONFIGURATION_VERSION = 0;
static constexpr unsigned int IMAGE_TILE_SIZE = 64;

void thread_shut_down(int id) {
    SortStatsFlushData();
}

void ImageEvaluation::StartRunning(int argc, char** argv) {
    m_image_title = "sort_" + logTimeString() + ".exr";

    // parse command arugments first
    parseCommandArgs(argc, argv);

    // create tsl thread context
    CreateTSLThreadContexts();

    // load the file
    std::unique_ptr<IStreamBase> stream_ptr = std::make_unique<IFileStream>( m_input_file );
    auto& stream = *stream_ptr;
    
    // load configuration
    loadConfig(stream);

    // setup job system
    marl::Scheduler::Config cfg;
    cfg.setWorkerThreadCount(m_thread_cnt);
    cfg.setWorkerThreadShutdown(thread_shut_down);

    m_scheduler = std::make_unique<marl::Scheduler>(cfg);
    m_scheduler->bind();

    if (!m_blender_mode)
        m_render_target = std::make_unique<RenderTarget>(m_image_width, m_image_height);

    // Load materials from stream
    auto& mat_pool = MatManager::GetSingleton().ParseMatFile(stream, m_no_material_mode);

#ifdef ENABLE_MULTI_THREAD_SHADER_COMPILATION
    marl::WaitGroup build_mat_wait_group(mat_pool.size());
    for (auto& mat : mat_pool) {
        marl::schedule([&](MaterialBase* mat) {
            defer(build_mat_wait_group.done());
            mat->BuildMaterial();
        }, mat.get());
    }
#endif

    // Serialize the scene entities
    m_scene.LoadScene(stream);

    // display the image first
    DisplayManager::GetSingleton().ResolveDisplayServerConnection();
    if (DisplayManager::GetSingleton().IsDisplayServerConnected()) {
        std::shared_ptr<DisplayImageInfo> image_info = std::make_shared<DisplayImageInfo>();
        image_info->title = m_image_title;
        image_info->w = m_image_width;
        image_info->h = m_image_height;
        image_info->is_blender_mode = m_blender_mode;
        DisplayManager::GetSingleton().QueueDisplayItem(image_info);
    }

    SORT_STATS(sSamplePerPixel = m_sample_per_pixel);
    SORT_STATS(sThreadCnt = m_thread_cnt);

    // Create a WaitGroup with an initial count of numTasks.
    marl::WaitGroup accel_structure_done(1);
    marl::WaitGroup pre_processing_done(1);

    // build acceleration structure
    marl::schedule([&]() {
        // Decrement the WaitGroup counter when the task has finished.
        defer(accel_structure_done.done());

        // update preprocessing time
        SORT_STATS(TIMING_EVENT_STAT("", sPreprocessingTimeMS));

        // Build acceleration structures, commonly QBVH
        m_scene.BuildAccelerationStructure();
    });

    // pre-processing for integrators, like instant radiosity
    marl::schedule([&]() {
        // Decrement the WaitGroup counter when the task has finished.
        defer(pre_processing_done.done());

        // this has to be after the two acceleration structures construction to be done.
        accel_structure_done.wait();

        // get a render context
        auto pRc = pullRenderContext();

        // preprocessing for integrators
        m_integrator->PreProcess(m_scene, *pRc);

        // recycle the render context
        recycleRenderContext(pRc);
    });

    // get the number of total task
    const auto tilesize = IMAGE_TILE_SIZE;
    Vector2i tile_num = Vector2i((int)ceil(m_image_width / (float)tilesize), (int)ceil(m_image_height / (float)tilesize));

    // start tile from center instead of top-left corner
    Vector2i cur_pos(tile_num / 2);
    int cur_dir = 0;
    int cur_len = 0;
    int cur_dir_len = 1;
    const Vector2i dir[4] = { Vector2i(0 , -1) , Vector2i(-1 , 0) , Vector2i(0 , 1) , Vector2i(1 , 0) };

    // make sure preprocessing is done
    pre_processing_done.wait();

#ifdef ENABLE_MULTI_THREAD_SHADER_COMPILATION
    // make sure all materials are built already
    build_mat_wait_group.wait();
#endif

    m_timer.Reset();

    // at this point, we are starting to render stuff
    while (true) {
        // only process node inside the image region
        if (cur_pos.x >= 0 && cur_pos.x < tile_num.x && cur_pos.y >= 0 && cur_pos.y < tile_num.y) {
            Vector2i tl(cur_pos.x * tilesize, cur_pos.y * tilesize);
            Vector2i size((tilesize < (m_image_width - tl.x)) ? tilesize : (m_image_width - tl.x),
                (tilesize < (m_image_height - tl.y)) ? tilesize : (m_image_height - tl.y));

            // pre-processing for integrators, like instant radiosity
            ++m_tile_cnt;
            marl::schedule([&](const Vector2i& ori, const Vector2i& size) {
                // get a render context
                auto pRc = pullRenderContext();
                auto& rc = *pRc;

                // get camera
                auto camera = m_scene.GetCamera();

                auto sampler = std::make_unique<RandomSampler>();
                auto pixelSamples = std::make_unique<PixelSample[]>(m_sample_per_pixel);

                // request samples
                m_integrator->RequestSample(sampler.get(), pixelSamples.get(), m_sample_per_pixel);

                const bool display_server_connected = DisplayManager::GetSingleton().IsDisplayServerConnected();
                const bool need_refresh_tile = m_integrator->NeedRefreshTile();

                const auto total_pixel = size.x * size.y;
                std::shared_ptr<DisplayTile> display_tile;
                if (display_server_connected && need_refresh_tile) {
                    std::shared_ptr<DisplayTile> indicate_tile;
                    indicate_tile = std::make_shared<DisplayTile>();
                    indicate_tile->x = ori.x;
                    indicate_tile->y = ori.y;
                    indicate_tile->w = size.x;
                    indicate_tile->h = size.y;
                    indicate_tile->is_blender_mode = m_blender_mode;
                    indicate_tile->title = m_image_title;

                    const auto indication_intensity = 0.3f;
                    if (m_blender_mode) {
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
                    display_tile->x = ori.x;
                    display_tile->y = ori.y;
                    display_tile->w = size.x;
                    display_tile->h = size.y;
                    display_tile->title = m_image_title;
                    display_tile->is_blender_mode = m_blender_mode;

                    if (m_blender_mode) {
                        display_tile->m_data[0] = std::make_unique<float[]>(total_pixel * 4);
                    } else {
                        for (auto i = 0u; i < 3; ++i)
                            display_tile->m_data[i] = std::make_unique<float[]>(total_pixel);
                    }
                }

                Vector2i rb = ori + size;
                for (int i = ori.y; i < rb.y; i++) {
                    for (int j = ori.x; j < rb.x; j++) {
                        // reset the memory allocator so that the last sample could reuse memory
                        // otherwise, memory usage is linear to spp.
                        rc.Reset();

                        // generate samples to be used later
                        m_integrator->GenerateSample(sampler.get(), pixelSamples.get(), m_sample_per_pixel, m_scene, rc);

                        // the radiance
                        Spectrum radiance;

                        auto valid_pixel_cnt = m_sample_per_pixel;
                        for (unsigned k = 0; k < m_sample_per_pixel; ++k) {

                            // generate rays
                            auto r = camera->GenerateRay((float)j, (float)i, pixelSamples[k]);
                            // accumulate the radiance
                            auto li = m_integrator->Li(r, pixelSamples[k], m_scene, rc);
                            if (m_clampping > 0.0f)
                                li = li.Clamp(0.0f, m_clampping);

                            sAssert(li.IsValid(), GENERAL);

                            if (li.IsValid())
                                radiance += li;
                            else
                                --valid_pixel_cnt;
                        }

                        if (valid_pixel_cnt > 0)
                            radiance /= (float)valid_pixel_cnt;

                        if (!m_blender_mode)
                            m_render_target->SetColor(j, i, radiance);

                        // update the value if display server is connected
                        if (display_server_connected && need_refresh_tile) {
                            auto local_i = i - ori.y;
                            auto local_j = j - ori.x;

                            if (m_blender_mode) {
                                auto local_index = local_j + (size.y - 1 - local_i) * size.x;
                                display_tile->m_data[0][4 * local_index] = radiance[0];
                                display_tile->m_data[0][4 * local_index + 1] = radiance[1];
                                display_tile->m_data[0][4 * local_index + 2] = radiance[2];
                                display_tile->m_data[0][4 * local_index + 3] = 1.0f;
                            }
                            else {
                                auto local_index = local_j + local_i * size.x;
                                for (auto i = 0u; i < RGBSPECTRUM_SAMPLE; ++i)
                                    display_tile->m_data[i][local_index] = radiance[i];
                            }
                        }
                    }
                }

                // update display server if needed
                if (display_server_connected && need_refresh_tile)
                    DisplayManager::GetSingleton().QueueDisplayItem(display_tile);

                // we are done with this tile
                --m_tile_cnt;

                // we are done with the render context, recycle it
                recycleRenderContext(pRc);
            }, tl, size);
        }

        // turn to the next direction
        if (cur_len >= cur_dir_len) {
            cur_dir = (cur_dir + 1) % 4;
            cur_len = 0;
            cur_dir_len += 1 - cur_dir % 2;
        }

        cur_pos += dir[cur_dir];
        ++cur_len;

        if ((cur_pos.x < 0 || cur_pos.x >= tile_num.x) && (cur_pos.y < 0 || cur_pos.y >= tile_num.y))
            break;
    }
}

int ImageEvaluation::WaitForWorkToBeDone() {
    Timer timer;
    while (m_tile_cnt > 0) {
        if (UNLIKELY(m_integrator->NeedFullTargetRealtimeUpdate())) {
            // only update it every 1 second
            if (timer.GetElapsedTime() > 1000) {
                std::shared_ptr<FullTargetUpdate> di = std::make_shared<FullTargetUpdate>();
                di->title = m_image_title;
                di->w = m_image_width;
                di->h = m_image_height;
                di->is_blender_mode = m_blender_mode;
                DisplayManager::GetSingleton().QueueDisplayItem(di);
                timer.Reset();
            }
        }

        DisplayManager::GetSingleton().ProcessDisplayQueue(6);
        std::this_thread::yield();
    }

    DisplayManager::GetSingleton().ProcessDisplayQueue(-1);

    if (!m_blender_mode)
        m_render_target->Output("sort_" + logTimeStringStripped() + ".exr");

    const bool display_server_connected = DisplayManager::GetSingleton().IsDisplayServerConnected();
    if (display_server_connected) {
        // some integrator might need a final refresh
        if (UNLIKELY(m_integrator->NeedFinalUpdate())) {
            std::shared_ptr<FullTargetUpdate> di = std::make_shared<FullTargetUpdate>();
            di->title = m_image_title;
            di->is_blender_mode = m_blender_mode;
            DisplayManager::GetSingleton().QueueDisplayItem(di);
        }

        // terminator is only needed in blender mode
        if (m_blender_mode) {
            std::shared_ptr<TerminateIndicator> terminator = std::make_shared<TerminateIndicator>();
            terminator->is_blender_mode = m_blender_mode;
            DisplayManager::GetSingleton().QueueDisplayItem(terminator);
        }

        // make sure flush all display items before quiting
        DisplayManager::GetSingleton().ProcessDisplayQueue(-1);
    }

    DestroyTSLThreadContexts();

    m_scheduler->unbind();
    m_scheduler = nullptr;

    SORT_STATS(sRenderingTimeMS = m_timer.GetElapsedTime());
    return 0;
}

void ImageEvaluation::parseCommandArgs(int argc, char** argv){
    // Parse command line arguments.
    const auto& args = parse_args(argc, argv, true);

    for (auto& arg : args) {
        const std::string& key_str = arg.first;
        const std::string& value_str = arg.second;

        if (key_str == "input") {
            m_input_file = value_str;
        }else if (key_str == "blendermode"){
            m_blender_mode = true;
        }else if (key_str == "profiling"){
            m_enable_profiling = value_str == "on";
        }else if (key_str == "nomaterial" ){
            m_no_material_mode = true;
        }else if (key_str == "displayserver") {
            int split = value_str.find_last_of(':');
            if (split < 0)
                continue;

            const auto ip = value_str.substr(0, split);
            const auto port = value_str.substr(split + 1);
            DisplayManager::GetSingleton().AddDisplayServer(ip, port);
        }
    }
}

void ImageEvaluation::loadConfig(IStreamBase& stream) {
    // check the version, there is no version for now, :(
    unsigned version = 0;
    stream >> version;
    sAssertMsg(GLOBAL_CONFIGURATION_VERSION == version, GENERAL, "Incompatible resource file with this version SORT.");

    stream >> m_resource_path;
    stream >> m_thread_cnt;
    stream >> m_sample_per_pixel;
    stream >> m_image_width >> m_image_height;
    stream >> m_clampping;

    StringID integratorType;
    stream >> integratorType;
    m_integrator = MakeUniqueInstance<Integrator>(integratorType);
    if (IS_PTR_VALID(m_integrator))
        m_integrator->Serialize(stream);
}