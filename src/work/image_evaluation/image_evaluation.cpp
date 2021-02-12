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
#include "core/log.h"

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

    m_need_render_target = !m_blender_mode || m_integrator->NeedFinalUpdate();
    if (m_need_render_target)
        m_render_target = std::make_unique<RenderTarget>(m_image_width, m_image_height);

    // Load materials from stream
    auto sc = pullContext(m_sc_holder);
    auto& mat_pool = MatManager::GetSingleton().ParseMatFile(stream, m_no_material_mode, sc->context.get());
    recycleContext(m_sc_holder, sc);

#ifdef ENABLE_MULTI_THREAD_SHADER_COMPILATION
    marl::WaitGroup build_mat_wait_group(mat_pool.size());
    for (auto& mat : mat_pool) {
        marl::schedule([&](MaterialBase* mat) {
            defer(build_mat_wait_group.done());

            auto sc = pullContext(m_sc_holder);
            mat->BuildMaterial(sc->context.get());
            recycleContext(m_sc_holder, sc);
        }, mat.get());
    }
#endif

    // Serialize the scene entities
    m_scene.LoadScene(stream);

    // display the image first
    DisplayManager::GetSingleton().ResolveDisplayServerConnection();
    if (DisplayManager::GetSingleton().IsDisplayServerConnected()) {
        std::shared_ptr<DisplayImageInfo> image_info = std::make_shared<DisplayImageInfo>(m_image_title, m_image_width, m_image_height, m_blender_mode);
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
        auto pRc = pullContext(m_rc_holder);

        // preprocessing for integrators
        m_integrator->PreProcess(m_scene, *pRc);

        // recycle the render context
        recycleContext(m_rc_holder, pRc);
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
            marl::schedule([this](const Vector2i& ori, const Vector2i& size) {
                // get a render context
                auto pRc = pullContext(m_rc_holder);
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
                    // indicate that we are rendering this tile
                    std::shared_ptr<IndicationTile> indicate_tile = std::make_shared<IndicationTile>(m_image_title, ori.x, ori.y, size.x, size.y, m_blender_mode);
                    DisplayManager::GetSingleton().QueueDisplayItem(indicate_tile);

                    display_tile = std::make_shared<DisplayTile>(m_image_title, ori.x, ori.y, size.x, size.y, m_blender_mode);
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

                        if (m_need_render_target)
                            UpdateImage(Vector2i(j,i), radiance);

                        // update the value if display server is connected
                        if (display_server_connected && need_refresh_tile) {
                            auto local_i = i - ori.y;
                            auto local_j = j - ori.x;
                            display_tile->UpdatePixel(local_j, local_i, radiance);
                        }
                    }
                }

                // update display server if needed
                if (display_server_connected && need_refresh_tile)
                    DisplayManager::GetSingleton().QueueDisplayItem(display_tile);

                // we are done with this tile
                --m_tile_cnt;

                // we are done with the render context, recycle it
                recycleContext(m_rc_holder, pRc);
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
                std::shared_ptr<FullTargetUpdate> di = std::make_shared<FullTargetUpdate>(m_image_title, m_render_target.get(), m_blender_mode);
                DisplayManager::GetSingleton().QueueDisplayItem(di);
                timer.Reset();
            }
        }

        DisplayManager::GetSingleton().ProcessDisplayQueue(6);
        std::this_thread::yield();
    }

    const bool display_server_connected = DisplayManager::GetSingleton().IsDisplayServerConnected();
    if (display_server_connected) {
        // some integrator might need a final refresh
        if (UNLIKELY(m_integrator->NeedFinalUpdate())) {
            std::shared_ptr<FullTargetUpdate> di = std::make_shared<FullTargetUpdate>(m_image_title, m_render_target.get(), m_blender_mode);
            DisplayManager::GetSingleton().QueueDisplayItem(di);
        }

        // send out the terminator indicator
        std::shared_ptr<TerminateIndicator> terminator = std::make_shared<TerminateIndicator>(m_blender_mode);
        DisplayManager::GetSingleton().QueueDisplayItem(terminator);
    }

    if (!m_blender_mode)
        m_render_target->Output("sort_" + logTimeStringStripped() + ".exr");

    // make sure flush all display items before quiting
    DisplayManager::GetSingleton().ProcessDisplayQueue(-1);

    DestroyTSLThreadContexts();

    m_scheduler->unbind();
    m_scheduler = nullptr;

    SORT_STATS(sRenderingTimeMS = m_timer.GetElapsedTime());

    // We have to wait after the disconnection of display server to move forward so that we are sure
    // all data has been passed through before SORT terminates itself.
    DisplayManager::GetSingleton().WaitForDisconnection(m_blender_mode);

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
    if (!m_thread_cnt)
        m_thread_cnt = std::thread::hardware_concurrency();

    stream >> m_sample_per_pixel;
    stream >> m_image_width >> m_image_height;
    stream >> m_clampping;

    StringID integratorType;
    stream >> integratorType;
    m_integrator = MakeUniqueInstance<Integrator>(integratorType);
    if (IS_PTR_VALID(m_integrator)) {
        m_integrator->Serialize(stream);
        m_integrator->SetImageEvaluation(this);
    }

    slog(INFO, GENERAL, "There will be %d threads rendering at the same time.", m_thread_cnt);
}

void ImageEvaluation::UpdateImage(const Vector2i& coord, const Spectrum& value) {
    if (m_integrator->NeedImageLock()) {
        std::lock_guard<std::mutex> guard(m_image_lock);
        const auto total = value + m_render_target->GetColor(coord.x, coord.y);
        m_render_target->SetColor(coord.x, coord.y, total);
    } else {
        m_render_target->SetColor(coord.x, coord.y, value);
    }
}