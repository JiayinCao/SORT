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

import bpy
import platform
from datetime import datetime

# This file may be extended with more sophiscated implementation in the future.
# However, it won't get upgraded unless it is needed to be complex.

# Output a log to blender terminal
def log(s):
    if platform.system() == 'Windows':
        header = '[' + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + '][SORT Plugin] '
        print( header + '\t' + s )
    else:
        header = '[' + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + '][SORT Plugin] '
        print( "\033[32m" + header + "\033[39m" + '\t' + s )

# Output more detailed log to blender terminal
def logD(s):
    if bpy.context.scene.sort_data.detailedLog is False:
        return
    log(s)
