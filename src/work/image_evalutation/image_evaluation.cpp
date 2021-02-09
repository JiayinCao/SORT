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
#include "image_evaluation.h"
#include "core/display_mgr.h"
#include "stream/fstream.h"

void ImageEvaluation::StartRunning(int argc, char** argv) {
    // parse command arugments first
    parseCommandArgs(argc, argv);

    // load the file
    IFileStream stream( m_input_file );
    
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