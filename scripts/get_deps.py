#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.
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
import shutil
import urllib.request
import zipfile
import subprocess

# syncing script url
get_dep_py_url = 'https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/get_deps.py'

# dependencies folder
dep_dir = 'dependencies'

# whether to force syncing
arch = 'x86_64'

# by default, we don't sync
sync_dep = False

# by default, don't false syncing anything
forcing_sync = False

# setup the architecture
if len(sys.argv) > 1:
    # output a message indicating this is a force syncing
    print( 'Force syncing dependencies.' )

    if sys.argv[1] == 'TRUE':
        forcing_sync = sys.argv[1]

    if len(sys.argv) > 2:
        if sys.argv[2] == 'arm64':
            arch = sys.argv[2]

# sync a dependency from server
def sync_dep_utility( url ):
    # get the syncing script first
    syncing_script_file_path = './get_deps.py'
    urllib.request.urlretrieve(get_dep_py_url, syncing_script_file_path)

    # sync the dependencies
    cmd = sys.executable + ' ' + syncing_script_file_path + ' ' + url + ' ' + dep_dir
    os.system(cmd)

    # remove the useless syncing script
    os.remove(syncing_script_file_path)

# fetch tsl dependency
def get_deps():
    # sync embree
    if sys.platform == 'win32':
        sync_dep_utility('https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/dep_win.txt')
    elif sys.platform == "linux" or sys.platform == "linux2":
        stream = os.popen('lsb_release -sc')
        output = stream.read()
        if output.find('xenial') != -1:
            sync_dep_utility('https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/dep_ubuntu_xenial.txt')
        elif output.find('focal') != -1:
            sync_dep_utility('https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/dep_ubuntu_focal.txt')
        elif output.find('bionic') != -1:
            sync_dep_utility('https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/dep_ubuntu_bionic.txt')
        else:
            print('Error. Embree on this platform is not pre-built, you will have to build it yourself.')
    elif sys.platform == 'darwin':
        if arch == 'arm64':
            print('Sycning arm version embree...')
            sync_dep_utility('https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/dep_mac_arm64.txt')
        elif arch == 'x86_64':
            print('Syncing x86_64 version embree...')
            sync_dep_utility('https://raw.githubusercontent.com/JiayinCao/ProjectDependencies/main/SORT/dep_mac_intel.txt')
        else:
            print('Error, unknown archtecture!')

# entry function
if __name__ == "__main__":
    # if forcing syncing is enabled, delete the dependencies even if it exists
    if forcing_sync:
        # check if the folder already exists, if it does, remove it
        if os.path.isdir(dep_dir):
            # output a warning
            print('The dependencies are purged.')

            # remove the folder
            shutil.rmtree(dep_dir)

        sync_dep = True
    else:
        # this might not be very robust since it just check the folder
        # if there is a broken dependencies folder, it will fail to build
        if os.path.isdir(dep_dir) is False:
            sync_dep = True
        else:
            print('Dependencies are up to date, no need to sync.')

    # if we need to sync, do it
    if sync_dep:
        get_deps()