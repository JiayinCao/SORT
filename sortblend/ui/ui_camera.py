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
import bl_ui
from .. import SORTAddon
from extensions_framework import declarative_property_group

# attach customized properties in camera
@SORTAddon.addon_register_class
class sort_camera(declarative_property_group):
    ef_attach_to = ['Camera']
    controls = []
    visibility = {}
    properties = []

@SORTAddon.addon_register_class
class sort_camera_lens(declarative_property_group):
    ef_attach_to = ['sort_camera']
    controls = []
    properties = [{ 'type': 'float',
                    'attr': 'lens_size',
                    'name': 'Size of Camera Lens',
                    'description': 'The size of lens in camera',
                    'default': 0.0,
                    'min': 0.0,
                    'soft_min': 0.0,
                    'max': 1e3,
                    'soft_max': 1e3,
                    'save_in_preset': True }]

class SORTCameraPanel(bl_ui.properties_data_camera.CameraButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    COMPAT_ENGINES = {'SORT_RENDERER'}
    @classmethod
    def register(cls):
        bl_ui.properties_data_camera.DATA_PT_lens.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_data_camera.DATA_PT_camera.COMPAT_ENGINES.add('SORT_RENDERER')
    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return super().poll(context) and rd.engine in cls.COMPAT_ENGINES

class CameraDOFPanel(SORTCameraPanel, bpy.types.Panel):
    bl_label = 'Camera Depth of Field'
    def draw(self, context):
        layout = self.layout
        camera = context.camera
        layout.prop(camera, "dof_object")
        row = layout.row()
        row.active = ( camera.dof_object == None )
        row.prop(camera, "dof_distance")
        layout.prop(camera.sort_camera.sort_camera_lens, "lens_size")