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
import shutil
import glob
import sys
from file_generator import fabric_brdf_lut, multi_scattering_lut

# the current directory is the sort root
sort_dir = os.getcwd()

# set the current directory as the generated source directory
sort_generated_src_dir = sort_dir + '/generated_src'

# if there is a second argument check what it is
allowremoving = False
if len(sys.argv) > 1:
    if sys.argv[1] == "force":
        allowremoving = True

# check if the folder already exists, if it does, remove it
if os.path.isdir(sort_generated_src_dir):
    if allowremoving:
        print('Source files are being regenerated.')

        files = glob.glob(sort_generated_src_dir + '/*')
        for f in files:
            os.remove(f)
else:
    # generate a new directory
    os.makedirs(sort_generated_src_dir)

# use the new directory as working directory for this script
os.chdir(sort_generated_src_dir)

# generate fabric lut
fabric_brdf_lut.generate()

# generate multi-scattering lut
multi_scattering_lut.generate()

# restore working directory
os.chdir(sort_dir)
