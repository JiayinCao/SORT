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
import os
from . import base
from . import renderer
from . import material
from .ui import ui_render
from .ui import ui_particle
from .ui import ui_world
from .ui import ui_camera
from .ui import ui_light
from .ui import ui_material

@base.register_class
class SORTAddonPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__
    install_path : bpy.props.StringProperty( name="Path to SORT binary", description='Path to SORT binary', subtype='DIR_PATH')

    def draw(self, context):
        self.layout.prop(self, "install_path")

def register():
    # register all classes in this plugin
    base.register()

    # this is the place for initializing group node information saved last time
    bpy.app.handlers.load_post.append(material.node_groups_load_post)

def unregister():
    # unregister everything already registered
    base.unregister()
