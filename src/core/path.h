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

#pragma once

#include "core/define.h"
#include <stdio.h>
#include <string.h>
#ifdef SORT_IN_WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include "core/globalconfig.h"

//! @brief  Full path of files in the resource folder.
//!
//! This is just a very simple function that appends the resource folder to the filename.
//! It is the caller's responsibilty to make sure the file is in the resource folder.
//!
//! @param filename     Name of the file in the resource folder.
//! @return             Full path that has the file name.
inline std::string GetFilePathInResourceFolder( const std::string& filename ){
    return g_resourcePath + filename;
}

//! @brief  Full path of files in the executable folder.
//!
//! This is just a very simple function that appends the executable folder to the filename.
//! It is the caller's responsibilty to make sure the file is in the executable folder.
//!
//! @param filename     Name of the file in the executable folder.
//! @return             Full path that has the file name.
inline std::string GetFilePathInExeFolder( const std::string& filename ){
    char buff[FILENAME_MAX];
    const char* tmp = GetCurrentDir( buff, FILENAME_MAX );
#ifdef WINDOWS
    return std::string(tmp) + "\\" + filename;
#else
    return std::string(tmp) + "/" + filename;
#endif
}
