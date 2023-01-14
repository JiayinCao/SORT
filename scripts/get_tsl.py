#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.
#
#    SORT is a free software written for educational purpose. Anyone can distribute
#    or modify it under the the terms of the GNU General Public License Version 3 as
#    published by the Free Software Foundation. However, there is NO warranty that
#    all components are functional in a perfect manner. Without even the implied
#    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along with
#    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
#

import os
import sys
import urllib.request
import zipfile
import subprocess

# sync a dependency from server
def sync_dep_utility( name, url ):
    print( 'Syncing ' + name )

    # dependencies path
    dep_path = 'dependencies/tsl'
    # target file path
    temp_zip_file = 'dependencies/tsl.zip'

    # get it from the server
    urllib.request.urlretrieve(url, temp_zip_file)

    # extract the file
    with zipfile.ZipFile(temp_zip_file,"r") as zip_ref:
        zip_ref.extractall(dep_path)

    # make sure the file is deleted
    os.remove(temp_zip_file)

# fetch tsl dependency
def get_dep_tsl(arch):
    # sync marl
    if sys.platform == 'win32':
        sync_dep_utility('tsl', 'https://github.com/JiayinCao/Tiny-Shading-Language/releases/download/Release-1.0.1/tsl_win.zip')
    elif sys.platform == "linux" or sys.platform == "linux2":
        sync_dep_utility('tsl', 'https://github.com/JiayinCao/Tiny-Shading-Language/releases/download/Release-1.0.1/tsl_linux.zip')
    elif sys.platform == 'darwin':
        if arch == 'arm64':
            print('Sycning arm version tsl...')
            sync_dep_utility('tsl', 'https://github.com/JiayinCao/Tiny-Shading-Language/releases/download/Release-1.0.1/tsl_apple_arm64.zip')
        elif arch == 'x86_64':
            print('Syncing x86_64 version tsl...')
            sync_dep_utility('tsl', 'https://github.com/JiayinCao/Tiny-Shading-Language/releases/download/Release-1.0.1/tsl_mac_intel.zip')
        else:
            print('Error, unknown archtecture!')
