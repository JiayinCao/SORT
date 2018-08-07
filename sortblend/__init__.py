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

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao",
    "version": (0, 0, 1),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

from extensions_framework import Addon
SORTAddon = Addon(bl_info)
addon_register, addon_unregister = SORTAddon.init_functions()

import bpy
from . import base
from . import renderer
from .ui import ui_render
from .ui import ui_camera
from .ui import ui_lamp
from .ui import ui_material

class SORTAddonPreferences(bpy.types.AddonPreferences):
    bl_idname = 'sortblend'

    # this must match the addon name
    install_path = bpy.props.StringProperty(
            name="Path to SORT binary",
            description='Path to SORT binary',
            subtype='DIR_PATH',
            default='',)

    pbrt_export_path = bpy.props.StringProperty(
            name='Pbrt exporting path',
            description='Path to exported pbrt scene',
            subtype='DIR_PATH',
            default='')

    def draw(self, context):
        self.layout.prop(self, "install_path")
        self.layout.prop(self, "pbrt_export_path")

def register():
    addon_register()
    bpy.utils.register_module(__name__)
    
def unregister():
    addon_unregister()
    bpy.utils.unregister_module(__name__)