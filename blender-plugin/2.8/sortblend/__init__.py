#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
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
    "description": "An open-source physically based renderer",
    "author": "Jiayin Cao",
    "version": (1, 0, 0),
    "blender": (2, 80, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

import bpy
from . import base
from . import renderer
from .ui import ui_render
#from .ui import ui_camera
#from .ui import ui_light
from .ui import ui_material
#from .ui import ui_particle
#from .ui import ui_space
from .material import nodes
from .material import properties

@base.register_class
class SORTAddonPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__
    install_path : bpy.props.StringProperty( name="Path to SORT binary", description='Path to SORT binary', subtype='DIR_PATH', default='',)
    def draw(self, context):
        self.layout.prop(self, "install_path")

def register():
    base.register()

    from bl_ui import (
            properties_data_light,
            )
    properties_data_light.DATA_PT_EEVEE_light.COMPAT_ENGINES.add( renderer.SORTRenderEngine.bl_idname )

def unregister():
    base.unregister()

    from bl_ui import (
            properties_data_light,
            )
    properties_data_light.DATA_PT_EEVEE_light.COMPAT_ENGINES.remove(renderer.SORTRenderEngine.bl_idname)

if __name__ == "__main__":
    register()