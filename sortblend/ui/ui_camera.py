import bpy
import bl_ui
from .. import common

def register():
    # register UI component
    bl_ui.properties_data_camera.DATA_PT_lens.COMPAT_ENGINES.add(common.renderer_bl_name)
    bl_ui.properties_data_camera.DATA_PT_camera_dof.COMPAT_ENGINES.add(common.renderer_bl_name)
    bl_ui.properties_data_camera.DATA_PT_camera.COMPAT_ENGINES.add(common.renderer_bl_name)

#def unregister():