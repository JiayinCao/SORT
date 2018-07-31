#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
# 
#    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
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
from . import ui_render
from . import ui_camera
from . import ui_lamp
from . import ui_material

def get_panels():
    types = bpy.types
    panels = [
        "DATA_PT_lens",
        "DATA_PT_camera",
        "RENDER_PT_dimensions",
        ]

    return [getattr(types, p) for p in panels if hasattr(types, p)]

def register():
    for panel in get_panels():
        panel.COMPAT_ENGINES.add('sortblend')
        
def unregister():
    for panel in get_panels():
        panel.COMPAT_ENGINES.add('sortblend')