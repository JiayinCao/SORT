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

# the current directory is the sort root
sort_dir = os.getcwd()

# dependencies folder
dep_dir = sort_dir + '/dependencies'

# tsl source target folder
tsl_dir = dep_dir + '/tsl_src'

# sync tsl code from github
os.system('git clone https://github.com/JiayinCao/Tiny-Shading-Language.git ' + tsl_dir)

# build tsl from source code
os.chdir(tsl_dir)
os.system('make install')
os.chdir(sort_dir)

# remove the tsl source dir
os.system('rm -rf ' + tsl_dir)