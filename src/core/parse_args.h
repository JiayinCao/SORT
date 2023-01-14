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

#pragma once

#include <string>
#include <vector>
#include <regex>
#include "core/profile.h"
#include "core/log.h"

inline std::vector<std::pair<std::string, std::string>> parse_args(int argc, char** argv, bool avoid_print_args = false) {
    SORT_PROFILE("Parse Commandline");

    std::string commandline = "Command line arguments: \t";
    for (int i = 0; i < argc; ++i) {
        commandline += std::string(argv[i]);
        commandline += " ";
    }
    if(!avoid_print_args)
        slog(INFO, GENERAL, "%s", commandline.c_str());

    std::vector < std::pair<std::string, std::string>> ret;

    std::regex word_regex("--(\\w+)(?:\\s*:\\s*([^ \\n]+)\\s*)?");
    auto words_begin = std::sregex_iterator(commandline.begin(), commandline.end(), word_regex);
    for (std::sregex_iterator it = words_begin; it != std::sregex_iterator(); ++it) {
        const auto m = *it;
        std::string key_str = m[1];
        std::string value_str = m.size() >= 3 ? std::string(m[2]) : "";

        // not case sensitive for key, but it is for value.
        std::transform(key_str.begin(), key_str.end(), key_str.begin(), ::tolower);

        ret.push_back(std::make_pair(key_str, value_str));
    }

    return ret;
}