#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
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
from bl_ui import properties_data_camera
from .. import base

# attach customized properties to particles
@base.register_class
class SORTCameraData(bpy.types.PropertyGroup):
    lens_size : bpy.props.FloatProperty( name='Lens Size', default=0.0)
    @classmethod
    def register(cls):
        bpy.types.Camera.sort_data = bpy.props.PointerProperty(name="SORT Data", type=cls)
    @classmethod
    def unregister(cls):
        del bpy.types.Camera.sort_data

class SORTCameraPanel(properties_data_camera.CameraButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    COMPAT_ENGINES = {'SORT'}
    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return super().poll(context) and rd.engine in cls.COMPAT_ENGINES

@base.register_class
class CAMERA_PT_SORTDOFPanel(SORTCameraPanel, bpy.types.Panel):
    bl_label = 'Camera Depth of Field'
    def draw(self, context):
        layout = self.layout
        camera = context.camera
        layout.prop(camera.dof, "focus_object")
        row = layout.row()
        row.active = ( camera.dof.focus_object == None )
        row.prop(camera.dof, "focus_distance")
        layout.prop(camera.sort_data, "lens_size")
