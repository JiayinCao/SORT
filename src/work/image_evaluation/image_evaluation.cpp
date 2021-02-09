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
#include <marl/scheduler.h>
#include <marl/waitgroup.h>
#include "image_evaluation.h"
#include "core/display_mgr.h"
#include "stream/fstream.h"
#include "core/strid.h"
#include "material/matmanager.h"

static constexpr unsigned int GLOBAL_CONFIGURATION_VERSION = 0;
static constexpr unsigned int IMAGE_TILE_SIZE = 64;

void ImageEvaluation::StartRunning(int argc, char** argv) {
    // parse command arugments first
    parseCommandArgs(argc, argv);

    // create tsl thread context
    CreateTSLThreadContexts();

    // load the file
    std::unique_ptr<IStreamBase> stream_ptr = std::make_unique<IFileStream>( m_input_file );
    auto& stream = *stream_ptr;
    
    // load configuration
    loadConfig(stream);

    // Load materials from stream
    MatManager::GetSingleton().ParseMatFile(stream);

    // Serialize the scene entities
    m_scene.LoadScene(stream);

    // display the image first
    DisplayManager::GetSingleton().ResolveDisplayServerConnection();
    if (DisplayManager::GetSingleton().IsDisplayServerConnected()) {
        std::shared_ptr<DisplayImageInfo> image_info = std::make_shared<DisplayImageInfo>();
        image_info->title = IMAGE_TILE_SIZE; // just hard code it for now
        image_info->w = m_image_width;
        image_info->h = m_image_height;
        DisplayManager::GetSingleton().QueueDisplayItem(image_info);
    }

    // setup job system
    marl::Scheduler::Config cfg;
    cfg.setWorkerThreadCount(m_thread_cnt);

    marl::Scheduler scheduler(cfg);
    scheduler.bind();
    defer(scheduler.unbind());  // Automatically unbind before returning.

    // Create a WaitGroup with an initial count of numTasks.
    marl::WaitGroup accel_structure_done(2);
    marl::WaitGroup pre_processing_done(1);

    // build acceleration structure
    marl::schedule([&]() {
        // Decrement the WaitGroup counter when the task has finished.
        defer(accel_structure_done.done());

        sAssert(m_accelerator, SPATIAL_ACCELERATOR);
        m_accelerator->Build(m_scene.GetPrimitives(), m_scene.GetBBox());
    });

    // bulid acceleration structure for volumes
    marl::schedule([&]() {
        // Decrement the WaitGroup counter when the task has finished.
        defer(accel_structure_done.done());

        sAssert(m_accelerator_vol, SPATIAL_ACCELERATOR);
        m_accelerator_vol->Build(m_scene.GetPrimitivesVol(), m_scene.GetBBoxVol());
    });

    // pre-processing for integrators, like instant radiosity
    marl::schedule([&]() {
        // Decrement the WaitGroup counter when the task has finished.
        defer(pre_processing_done.done());

        // this has to be after the two acceleration structures construction to be done.
        accel_structure_done.wait();

        // get a render context
        auto& rc = pullRenderContext();

        sAssert(m_accelerator_vol, SPATIAL_ACCELERATOR);
        m_integrator->PreProcess(m_scene, rc);
    });
}

int ImageEvaluation::WaitForWorkToBeDone() {
    return 0;
}

void ImageEvaluation::parseCommandArgs(int argc, char** argv){
    std::string commandline = "Command line arguments: \t";
    for (int i = 0; i < argc; ++i) {
        commandline += std::string(argv[i]);
        commandline += " ";
    }
    slog( INFO , GENERAL , "%s" , commandline.c_str() );

    bool com_arg_valid = false;
    std::regex word_regex("--(\\w+)(?:\\s*:\\s*([^ \\n]+)\\s*)?");
    auto words_begin = std::sregex_iterator(commandline.begin(), commandline.end(), word_regex);
    for (std::sregex_iterator it = words_begin; it != std::sregex_iterator(); ++it) {
        const auto m = *it;
        std::string key_str = m[1];
        std::string value_str = m.size() >= 3 ? std::string(m[2]) : "";

        // not case sensitive for key, but it is for value.
        std::transform(key_str.begin(), key_str.end(), key_str.begin(), ::tolower);

        if (key_str == "input") {
            m_input_file = value_str;
            com_arg_valid = true;
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
    std::string dummy_str;
    stream >> dummy_str;

    // this doesn't need to come from input at all
    unsigned dummy_tile_size;
    stream >> dummy_tile_size;

    stream >> m_thread_cnt;
    stream >> m_sample_per_pixel;
    stream >> m_image_width >> m_image_height;
    stream >> m_clampping;

    StringID accelType, integratorType;
    stream >> accelType;
    m_accelerator = MakeUniqueInstance<Accelerator>(accelType);
    if (m_accelerator)
        m_accelerator->Serialize(stream);
    m_accelerator_vol = std::move(m_accelerator->Clone());

    stream >> integratorType;
    m_integrator = MakeUniqueInstance<Integrator>(integratorType);
    if (IS_PTR_VALID(m_integrator))
        m_integrator->Serialize(stream);
}