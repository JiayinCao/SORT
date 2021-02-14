#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
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
import shutil
import sys
import get_tsl
import get_marl

# whether to force syncing
forcing_sync = False
arch = 'x86_64'

if len(sys.argv) > 1:
    # output a message indicating this is a force syncing
    print( 'Force syncing dependencies.' )

    if sys.argv[1] == 'TRUE':
        forcing_sync = sys.argv[1]

    if len(sys.argv) > 2:
        if sys.argv[2] == 'arm64':
            arch = sys.argv[2]

# dependencies folder
dep_dir = 'dependencies'

# whether to sync dependencies
sync_dep = False

# sync pre-compiled tsl by default
build_tsl = False

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
    
# sync dependencies if needed
if sync_dep:
    # create the directory first
    os.makedirs(dep_dir)

    # TSL
    if build_tsl:
        exec(open("./scripts/build_tsl.py").read())
    else:
        get_tsl.get_dep_tsl(arch)

    # marl
    get_marl.get_dep_marl(arch)
